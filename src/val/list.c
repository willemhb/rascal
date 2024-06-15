#include "val/list.h"


/* External APIs */
/* List API */
/* Pair API */
Pair* mk_pair(Val a, Val d) {
  Pair* p = new_obj(&PairType);
  p->car  = a;
  p->cdr  = d;

  return p;
}

/* MList API */
