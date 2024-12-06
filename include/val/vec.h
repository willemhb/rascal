#ifndef rl_val_vec_h
#define rl_val_vec_h

#include "val/object.h"

/* C types */
struct Vec {
  HEADER;

  size64 cnt;
  VNode* rt;
  Val*   tl;
};

/* Globals */
extern Vec EmptyVec;

/* External APIs */
#define is_vec(x) has_type(x, T_VEC)
#define as_vec(x) ((Vec*)as_obj(x))

Vec* mk_vec(size64 n, Val* vs);
Val  vec_ref(Vec* v, size64 n);
Vec* vec_add(Vec* v, Val x);
Vec* vec_set(Vec* v, size64 i, Val x);
Vec* vec_pop(Vec* v, Val* r);
Vec* vec_cat(Vec* vx, Vec* vy);

// initialization
void rl_toplevel_init_vec(void);

#endif
