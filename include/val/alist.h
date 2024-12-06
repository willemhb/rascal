#ifndef rl_val_alist_h
#define rl_val_alist_h

#include "val/object.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
struct Alist {
  HEADER;

  size64 cnt, cap;

  Val* data;
};

/* Globals */

/* External APIs */
#define is_alist(x) has_type(x, T_ALIST)
#define as_alist(x) ((Alist*)as_obj(x))

// mostly internal methods
Alist* new_alist(void);
void   init_alist(Alist* a);
void   shrink_alist(Alist* a, size64 n);
void   grow_alist(Alist* a, size64 n);
void   resize_alist(Alist* a, size64 n);

// external methods
Alist* mk_alist(size64 n, Val* d);
Val    alist_ref(Alist* a, size64 n);
size64 alist_add(Alist* v, Val x);
size64 alist_wrt(Alist* a, size64 n, Val* d);
void   alist_set(Alist* v, size64 n, Val x);
Val    alist_pop(Alist* a);
Val    alist_popn(Alist* a, size64 n, bool e);
void   alist_cat(Alist* x, Alist* y);

/* Initialization */

#endif
