#include <string.h>

#include "rascal.h"

// runtime system
#include "runtime/memory.h"
#include "runtime/stack.h"
#include "runtime/init.h"
#include "runtime/error.h"

// primtives
#include "object/binary.h"
#include "object/boolean.h"
#include "object/character.h"
#include "object/integer.h"

// objects
#include "object/error.h"
#include "object/function.h"
#include "object/pair.h"
#include "object/port.h"
#include "object/string.h"
#include "object/symbol.h"
#include "object/table.h"
#include "object/tuple.h"
#include "object/type.h"

// virtual machine
#include "vm/apply.h"
#include "vm/builtin.h"
#include "vm/compile.h"
#include "vm/eval.h"
#include "vm/exec.h"
#include "vm/print.h"
#include "vm/read.h"

// declare globals
ulong_t   Symcnt;
object_t *Symbols, *Globals, *Error, *Ins, *Outs, *Errs;

value_t Stack[N_STACK], Dump[N_STACK], Function;

index_t Sp, Bp, Fp, Dp;

uchar_t *Heap, *Reserve, *Free, *MapFree, *HeapMap, *ReserveMap;

size_t  NHeap, HeapUsed, HeapSize, NReserve, ReserveUsed, ReserveSize;

bool_t  Collecting, Grow, Grew;

float_t Collectf, Resizef, Growf;

// declare type dispatch tables
construct_t Construct[N_TYPES];
init_t      Init[N_TYPES];
trace_t     Trace[N_TYPES];
relocate_t  Relocate[N_TYPES];
untrace_t   Untrace[N_TYPES];
print_t     Print[N_TYPES];
finalize_t  Finalize[N_TYPES];
sizeof_t    Sizeof[N_TYPES];
mk_hash_t   Hash[N_TYPES];

char_t  *TypeNames[N_TYPES];
size_t   TypeSizes[N_TYPES];
uint_t   TypeFlags[N_TYPES];            // default flags for a type
bool_t   TypeMembers[N_TYPES][N_TYPES]; // simplefied subtyping

// declare instructions dispatch tables
Cbuiltin_t    Builtins[form_pad];
ensure_t      Ensure[form_pad];
size_t        InstructionArgC[num_instructions];
char_t       *InstructionNames[num_instructions];

// static initializers for dispatch tables
static void dispatch_init( void ) {
  // ensure all pointers without methods are NULL (no wild pointers)
  memset( Construct, 0, N_TYPES*sizeof(construct_t) );
  memset( Init, 0, N_TYPES*sizeof(init_t) );
  memset( Trace, 0, N_TYPES*sizeof(trace_t) );
  memset( Relocate, 0, N_TYPES*sizeof(relocate_t) );
  memset( Untrace, 0, N_TYPES*sizeof(untrace_t) );
  memset( Print, 0, N_TYPES*sizeof(print_t) );
  memset( Finalize, 0, N_TYPES*sizeof(finalize_t) );
  memset( Sizeof, 0, N_TYPES*sizeof(sizeof_t) );
  memset( Hash, 0, N_TYPES*sizeof(mk_hash_t) );
  memset( TypeNames, 0, N_TYPES*sizeof(char_t*) );
  memset( TypeSizes, 0, N_TYPES*sizeof(size_t) );
  memset( TypeFlags, 0, N_TYPES*sizeof(uint_t) );
  memset( TypeMembers, 0, N_TYPES*N_TYPES*sizeof(bool_t) );

  memset( Builtins, 0, form_pad*sizeof(Cbuiltin_t) );
  memset( Ensure, 0, form_pad*sizeof(ensure_t) );
  memset( InstructionArgC, 0, num_instructions*sizeof(size_t) );
  memset( InstructionNames, 0, num_instructions*sizeof(char_t*) );
}

static void fuckedup_types_init( void ) {
  // initialize the names of the weird types
  TypeNames[type_any] = "any";
  TypeSizes[type_any] = 8;       // since 'any' typically means 'any concrete value'

  TypeNames[type_none] = "none";


  // initialize the trival cells of TypeMembers
  for (int i=0; i<N_TYPES; i++) {
    TypeMembers[type_any][i] = true;
    TypeMembers[i][i] = true;
    TypeMembers[type_none][i] = false;
  }
}

static void rascal_init( void ) {
  memory_init( );
  dispatch_init( );
  fuckedup_types_init( );
  
}

int main( int argc, const char **argv ) {
  (void)argc;
  (void)argv;

  rascal_init( );

  return 0;
}
