// standard headers
#include <string.h>

// core
#include "rascal.h"

// runtime system
#include "runtime/memory.h"
#include "runtime/stack.h"
#include "runtime/init.h"
#include "runtime/error.h"
#include "runtime/object.h"

// virtual machine
#include "vm/apply.h"
#include "vm/compile.h"
#include "vm/exec.h"
#include "vm/lisp-io.h"

// declare globals
ulong   Symcnt = 0;

root_t Symbols, Syntax, Characters;
value_t Error, Ins, Outs, Errs;

value_t Stack[N_STACK], Dump[N_STACK];
index_t Sp = 0,
        Bp = 0,
        Fp = 0,
        Dp = 0;

uchar *Heap, *Reserve, *Free;

size_t  HeapSize = N_STACK*sizeof(cons_t),
        HeapUsed = 0,
        RSize    = N_STACK*sizeof(cons_t),
        RUsed    = 0;

bool  Collecting = false,
      Grow       = false,
      Grew       = false;

float Collectf   = 1.0,
      Resizef    = 0.685,
      Growf      = 2.0;

// type dispatch --------------------------------------------------------------
char    *TypeNames[TYPE_PAD]  = {
  [type_cons]      = "cons",      [type_symbol]   = "symbol",   [type_vector]  = "vector",
  [type_table]     = "table",     [type_string]   = "string",   [type_port]    = "port",
  [type_closure]   = "closure",

  [type_bytecode] = "bytecode",

  [type_none]      = "none",      [type_null]     = "null",     [type_type]    = "type",

  [type_fixnum]    = "fixnum",    [type_integer]  = "int",

  [type_character] = "char",      [type_boolean]  = "bool",

  [type_builtin]   = "builtin", 
};

size_t TypeSizes[TYPE_PAD]  = {
  [type_cons]      = sizeof(cons_t),    [type_symbol]   = sizeof(symbol_t),
  [type_vector]    = sizeof(vector_t),  [type_table]    = sizeof(node_t),
  [type_string]    = sizeof(string_t),  [type_port]     = sizeof(port_t),
  [type_closure]   = sizeof(closure_t),

  [type_dict]      = sizeof(dict_t),    [type_bytecode] = sizeof(bytecode_t),
  
  [type_none]      = 8,                 [type_null]     = 8,
  [type_any]       = 8,                 [type_type]     = 8,

  [type_fixnum]    = 8,                 [type_integer]  = sizeof(int),

  [type_character] = sizeof(char),      [type_boolean]  = sizeof(bool),
  [type_builtin]   = sizeof(short),     [type_form]     = sizeof(short),
  [type_opcode]    = sizeof(short)
};

type_t TypeEltype[TYPE_PAD] = {
  [type_vector]   = type_any, [type_string] = type_character,
  [type_bytecode] = type_opcode,
};

int     (*Sizeof[TYPE_PAD])( void *p, type_t t )               = { object_sizeof };
int     (*Relocate[TYPE_PAD])( void *p, type_t t, value_t *b ) = { object_relocate };
void    (*Trace[TYPE_PAD])( void *p, type_t t, int n )         = { object_trace };
int     (*Print[TYPE_PAD])( FILE *ios, value_t x )             = { value_print };
int     (*Order[TYPE_PAD])( value_t x, value_t y )             = { value_order };

// 
static void rascal_init( void ) {
  init_memory( );
  init_tables( );
  init_dispatch( );
  init_types( );
  init_names( );
  init_builtins( );
  init_globals( );
}

int main( int argc, const char **argv ) {
  (void)argc;
  (void)argv;

  rascal_init( );

  return 0;
}
