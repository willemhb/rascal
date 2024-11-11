#include "val/environ.h"
#include "val/list.h"
#include "val/text.h"
#include "val/array.h"
#include "val/table.h"

#include "vm/state.h"
#include "vm/type.h"

#include "util/hash.h"

/* Forward declarations */
// Sym methods
extern void trace_sym(State* vm, void* x);
extern bool egal_syms(Val x, Val y);
extern int  order_syms(Val x, Val y);

// Str methods
extern void free_str(State* vm, void* x);
extern bool egal_strs(Val x, Val y);
extern int  order_strs(Val x, Val y);

// list methods
extern void   trace_list(State* vm, void* x);
extern hash64 hash_list(Val x);
extern bool   egal_lists(Val x, Val y);
extern int    order_lists(Val x, Val y);

// Buffer methods
extern void free_buffer(State* vm, void* x);

// Alist methods
extern void trace_alist(State* vm, void* x);
extern void free_alist(State* vm, void* x);

// Table methods
extern void trace_table(State* vm, void* x);
extern void free_table(State* vm, void* x);

/* Globals */
VTable Vts[N_TYPES] = {
  [T_NUL]    = {
    .code    = T_NUL,
    .name    = "nul",
  },

  [T_SYM]    = {
    .code    = T_SYM,
    .name    = "symbol",
    .obsize  = sizeof(Sym),
    .tracefn = trace_sym,
    .egalfn  = egal_strs,
    .orderfn = order_strs,
  },

  [T_STR]    = {
    .code    = T_STR,
    .name    = "string",
    .obsize  = sizeof(Str),
    .freefn  = free_str,
    .egalfn  = egal_strs,
    .orderfn = order_strs,
  },

  [T_LIST]   = {
    .code    = T_LIST,
    .name    = "list",
    .obsize  = sizeof(List),
    .tracefn = trace_list,
    .hashfn  = hash_list,
    .egalfn  = egal_lists,
    .orderfn = order_lists,
  },
  
  [T_BUFFER] = {
    .code    = T_BUFFER,
    .name    = "buffer",
    .obsize  = sizeof(Buffer),
    .freefn  = free_buffer,
  },

  [T_ALIST]  = {
    .code    = T_ALIST,
    .name    = "alist",
    .obsize  = sizeof(Alist),
    .tracefn = trace_alist,
    .freefn  = free_alist,
  },
  
  [T_TABLE]  = {
    .code    = T_TABLE,
    .name    = "table",
    .obsize  = sizeof(Table),
    .tracefn = trace_table,
    .freefn  = free_table,
  },
};

/* External APIs */
void rl_init_types(State* vm) {
  for ( Type t = T_NUL; t < N_TYPES; t++ )
    vm->vts[t].hash = hash_word(t);
}

void rl_toplevel_init_types(void) {
  rl_init_types(&Vm);
}
