#ifndef rl_val_vec_h
#define rl_val_vec_h

#include "val/object.h"

#include "util/amt.h"

/* C types */
typedef struct {
  
} VecIter;

struct Vec {
  HEADER;

  size64 cnt;
  VNode* root;
  Val    tail[];
};

struct VNode {
  HEADER;

  size16 cnt, max, shift;

  Obj** children;
};

struct VLeaf {
  HEADER;

  Val vals[HT_MAXC];
};

struct MVec {
  HEADER;

  size64 cnt, cap;

  Val* data;
};

/* Globals */
extern Type VecType, VNodeType, VLeafType, MVecType;

/* APIs */
// Vec and supporting types
#define is_vec(x) has_type(x, T_VEC)
#define as_vec(x) ((Vec*)as_obj(x))

Vec* mk_vec(size64 n, Val* vs);
Val  vec_ref(Vec* v, size64 n);
Vec* vec_add(Vec* v, Val x);
Vec* vec_set(Vec* v, size64 i, Val x);
Vec* vec_pop(Vec* v, Val* r);
Vec* vec_cat(Vec* vx, Vec* vy);

#define is_vnode(x) has_type(x, T_VNODE)
#define as_vnode(x) ((VNode*)as_obj(x))

#define is_vleaf(x) has_type(x, T_VLEAF)
#define as_vleaf(x) ((VLeaf*)as_obj(x))

// MVec
#define is_mvec(x) has_type(x, T_MVEC)
#define as_mvec(x) ((MVec*)as_obj(x))

// mostly internal methods
MVec*  new_mvec(void);
void   init_mvec(MVec* a);
void   shrink_mvec(MVec* a, size64 n);
void   grow_mvec(MVec* a, size64 n);
void   resize_mvec(MVec* a, size64 n);

// external methods
MVec*  mk_mvec(size64 n, Val* d);
Val    mvec_ref(MVec* a, size64 n);
size64 mvec_add(MVec* v, Val x);
size64 mvec_wrt(MVec* a, size64 n, Val* d);
void   mvec_set(MVec* v, size64 n, Val x);
Val    mvec_pop(MVec* a);
Val    mvec_popn(MVec* a, size64 n, bool e);
void   mvec_cat(MVec* x, MVec* y);

// initialization
void rl_init_vec(void);

#endif
