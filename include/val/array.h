#ifndef rl_val_array_h
#define rl_val_array_h

#include "val/object.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
struct Alist {
  HEADER;

  size64 cnt, cap;

  Val* data;
};

struct Vec {
  HEADER;

  size64 cnt;
  VNode* rt;
  Val*   tl;
};

struct VNode {
  HEADER;

  bool full;         // indicates whether the node is full
  size16 cnt, shft;

  union {
    VNode** cn;
    Val*    vs;
  };
};

/* Globals */
extern Vec EmptyVec;

/* APIs */
// Alist API
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

// Vec API
#define is_vec(x) has_type(x, T_VEC)
#define as_vec(x) ((Vec*)as_obj(x))

Vec* mk_vec(size64 n, Val* vs);
Val  vec_ref(Vec* v, size64 n);
Vec* vec_add(Vec* v, Val x);

#endif
