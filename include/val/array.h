#ifndef rl_val_array_h
#define rl_val_array_h

#include "val/object.h"

#include "util/table.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
// vector root and node types
struct Vec {
  HEADER;

  // additional bit fields
  word_t algo   :  2;
  word_t packed :  1;

  // data fields
  Struct* type;

  union {
    size_t arity;

    struct {
      uint cnt;
      uint cap;
    };
  };

  VNode*  root;
  Val*    tail;
};

struct VNode {
  HEADER;

  ushort cnt, cap, shift;

  union {
    VNode** cn;
    Val*    slots;
  };
};

/* Globals */
/* APIs */
/* Vec API */
#define is_vec(x) has_type(x, &VecType)
#define as_vec(x) ((Vec*)as_obj(x))

Vec* mk_vec(size_t n, Val* d);
Vec* packed_vec(size_t n, Val* d);
Val  vec_ref(Vec* v, size_t n);
Vec* vec_add(Vec* v, Val x);
Vec* vec_set(Vec* v, size_t n, Val x);
Vec* vec_pop(Vec* x);
Vec* vec_cat(Vec* x, Vec* y);

/* VNode API (mostly internal) */
#define is_vnode(x) has_type(x, &VNodeType)
#define as_vnode(x) ((VNode*)as_obj(x))

#endif
