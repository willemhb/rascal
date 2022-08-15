// C modules ------------------------------------------------------------------
#include <string.h>
#include <assert.h>

#include "memutils.h"
#include "numutils.h"
#include "strutils.h"
#include "hashing.h"

#include "common.h"

// runtime modules ------------------------------------------------------------
#include "memory.h"
#include "value.h"

// object modules -------------------------------------------------------------
#include "number.h"
#include "character.h"
#include "boolean.h"
#include "pointer.h"
#include "list.h"
#include "symbol.h"
#include "function.h"
#include "table.h"
#include "array.h"
#include "environment.h"
#include "types.h"

// vm modules -----------------------------------------------------------------
#include "read.h"
#include "print.h"
#include "eval.h"
#include "exec.h"
#include "apply.h"

// globals --------------------------------------------------------------------
stack_t  *Stack;
heap_t   *Heap;
module_t *Module = NULL;

ulong_t     SymCount  = 0;
ulong_t     TypeCount = 0;
table_t    *Symbols;
gc_frame_t *Saved = NULL;

data_type_t *ImmediateTypes[N_BUILTIN_IMMEDIATE];
data_type_t *ObjectTypes[2][N_BUILTIN_OBJECT];

// backing objects for core VM objects ----------------------------------------
stack_t  StackObject rsp_aligned;
heap_t   HeapObject rsp_aligned;
module_t ToplevelObject rsp_aligned;
table_t  SymbolsObject rsp_aligned;

void init_stack(stack_t *ob);
void init_heap(heap_t *ob);
void init_symbols(table_t *ob);
void init_module(module_t *ob, char *name);

void init_global_objects(void) {
  Stack       = &StackObject;
  Heap        = &HeapObject;
  Module      = &ToplevelObject;
  Symbols     = &SymbolsObject;

  init_stack(Stack);
  init_heap(Heap);
  init_symbols(Symbols);
  init_module(Module, "<toplevel>");
}

// form names -----------------------------------------------------------------
value_t sym_quote, sym_do, sym_cntl, sym_hndl;
value_t sym_cond, sym_case, sym_if, sym_and, sym_or;
value_t sym_val, sym_fun, sym_mac, sym_let, sym_label;

/* runtime */
// allocation flags -----------------------------------------------------------


// type implementations -------------------------------------------------------

/* */

/* read/print */

/* interpreter */

/* compiler */

/* initialization */
void rascal_init(void) {
  init_global_objects();
  init_forms();
}

/* executable */
int main(int argc, char *argv[]) {
  rascal_init();
  
  (void)argc;
  (void)argv;

  return 0;
}
