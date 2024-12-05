#include "error.h"

#include "lang/native.h"
#include "lang/compare.h"

#include "val/environ.h"
#include "val/function.h"
#include "val/list.h"

#include "vm/state.h"

/* Globals */

/* Native functions */
// comparison
void pf_same(Proc* p, int n) {
  (void)n;

  Val y   = pop(p),
    x     = pop(p),
    r     = x == y ? TRUE : FALSE;
  *tos(p) = r; // TOS should be the caller, so we're saving a pop/push
}

void pf_egal(Proc* p, int n) {
  (void)n;

  Val y   = pop(p),
      x   = pop(p),
      r   = rl_egal(x, y) ? TRUE : FALSE;
  *tos(p) = r; // TOS should be the caller, so we're saving a pop/push
}

void pf_ord(Proc* p, int n) {
  (void)n;

  Val y   = pop(p), x = pop(p), r;
  int o   = rl_order(x, y);
  r       = tag(o);
  *tos(p) = r; // TOS should be the caller, so we're saving a pop/push
}

void pf_hash(Proc* p, int n) {
  (void)n;

  Val y   = pop(p),
      h   = rl_hash(y),
      r   = tag(h);
  *tos(p) = r; // TOS should be the caller, so we're saving a pop/push
}

// constructors
void pf_list(Proc* p, int n) {
  Val* b  = sref(p, -n);
  List* r = mk_list(n, b); popn(p, n, true);
  *tos(p) = tag(r); // TOS should be the caller, so we're saving a pop/push
}

