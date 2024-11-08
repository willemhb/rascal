#include "val/text.h"

#include "vm/type.h"

/* Forward declarations */
// Str methods
extern void free_str(State* vm, void* x);
extern bool egal_strs(Val x, Val y);
extern int order_strs(Val x, Val y);

/* Globals */
VTable Vts[N_TYPES] = {
  [T_STR]  = {
    .code    = T_STR,
    .obsize  = sizeof(Str),
    .freefn  = free_str,
    .egalfn  = egal_strs,
    .orderfn = order_strs,
  },
};
