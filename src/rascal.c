// system includes
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// toplevel includes
#include "types.h"
#include "val.h"
#include "obj.h"

// object includes
#include "array.h"
#include "pair.h"
#include "atom.h"
#include "envt.h"
#include "port.h"
#include "number.h"
#include "func.h"
#include "method.h"

// runtime includes
#include "memory.h"
#include "read.h"
#include "prin.h"
#include "exec.h"

// utility includes
#include "utils.h"

// globals
// VM singletons
symt_t  Symbols;
heap_t  Heap;
readt_t Reader;
envt_t  Toplevel;
typet_t Types;

// standard streams
stream_t Ins, Outs, Errs;

// famous symbols
value_t Quote, Error;

// dispatch tables
const char  *TypeNames[N_REPR] =
  {
    [REAL]    = "real",        [CHRTYPE] = "chr",   [NULTYPE] = "nul",
    [SYMBOL]    = "atom",        [SYMT]    = "symt",  [READT]   = "readt",
    [RENTRY]  = "readt-entry", [PORT]    = "port",  [CONS]    = "cons",
    [ALIST]   = "alist",       [STACK]   = "stack", [ENVT]    = "envt",
    [VAR]     = "var",         [CODE]    = "code",  [INSTR]   = "instr",
    [VM]      = "vm",

    [ANY]     = "any",         [NONE]    = "none",
  };

printer_fn_t Print[N_REPR] =
  {
   [REAL] = prin_real, [SYMBOL]   = prin_atom, [CHRTYPE] = prin_chr,
   [CONS] = prin_list, [NULTYPE]= prin_list,
  };

const size_t BaseSizes[N_REPR] =
  {
   [SYMBOL]  =sizeof(atom_t),     [CONS]   =sizeof(cons_t),
   [FUNCTION]=sizeof(function_t), [CONTROL]=sizeof(control_t),
   [BYTECODE]=sizeof(bytecode_t), [STREAM] =sizeof(stream_t),
   [BINARY]  =sizeof(binary_t),   [ALIST]  =sizeof(alist_t),
   [BUFFER]  =sizeof(buffer_t),   [ARRAY]  =sizeof(array_t),
   [TABLE]   =sizeof(table_t),    [ENTRY]  =sizeof(entry_t),
   [ASSOC]   =sizeof(assoc_t),    [VAR]    =sizeof(var_t),
   [BIGINT]  =sizeof(bigint_t),
  };

mark_fn_t Mark[N_REPR] =
  {
    [SYMBOL]  =mark_symbol,   [CONS]   =mark_cons,
    [FUNCTION]=mark_function, [CONTROL]=mark_control,
    [BYTECODE]=mark_bytecode, [STREAM] =mark_stream,
    [ALIST]   =mark_alist,    [ARRAY]  =mark_array,
    [TABLE]   =mark_table,    [ENTRY]  =mark_entry,
    [ASSOC]   =mark_assoc,    [VAR]    =mark_var,
    [RECORD]  =mark_record
  };

free_fn_t Free[N_REPR] =
  {
   [SYMBOL] =free_atom,   [PORT]  =free_port,
   [SYMT]   =free_symt,   [READT] =free_readt,
   [STACK]  =free_stack,  [ALIST] =free_alist,
   [BUFFER] =free_buffer,
  };

// initialization and some utilities

// reader
#define VERSION "%d.%d.%d.%c"
#define MAJOR   0
#define MINOR   1
#define PATCH   1
#define DEV     'a'

static void prin_welcome( void )
{
  printf("Welcome to rascal version "VERSION"!\n\n", MAJOR, MINOR, PATCH, DEV );
}

static void init_rascal( void )
{
  memory_init();
  atom_init();
  read_init();
  port_init();
  envt_init();
  exec_init();
  method_init();
}

int main(const int argc, const char *argv[argc])
{
  (void)argv;

  init_rascal();
  prin_welcome();
  lisp_repl();

  return 0;
}
