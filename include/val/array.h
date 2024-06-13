#ifndef rl_val_array_h
#define rl_val_array_h

#include "val/object.h"

#include "util/table.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
// vector root and node types
struct Vec {
  HEADER;

  // bit fields
  word_t shift  : 6;
  word_t trans  : 1;
  word_t packed : 1;

  // data fields
  size_t count;
  VNode* root;
  Val*   tail;
};

struct VNode {
  HEADER;

  // bit fields
  word_t shift : 6;
  word_t trans : 1;

  size_t count;

  // data fields
  union {
    VNode** children;
    Val*    slots;
  };
};

// dynamic array types
#define DYNAMIC_ARRAY(X)                        \
  word_t algo   : 2;                            \
  word_t shrink : 1;                            \
  X* data;                                      \
  X* _static;                                   \
  size_t count, max_count, max_static

struct MVec {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(Val);
};

struct Alist {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(void*);
};

#undef DYNAMIC_ARRAY

/* Globals */
// types
extern Type VecType, VNodeType, MVecType, AlistType;

/* APIs */
/* Vec API */
#define as_vec(x) ((Vec*)as_obj(x))
#define is_vec(x) has_type(x, &VecType)

Vec* mk_vec(size_t n, Val* d);
Vec* packed_vec(size_t n, Val* d);
Val  vec_ref(Vec* v, size_t n);
Vec* vec_add(Vec* v, Val x);
Vec* vec_set(Vec* v, size_t n, Val x);
Vec* vec_pop(Vec* v, Val* r);
Vec* vec_cat(Vec* x, Vec* y);

/* Dynamic array APIs */
#define MUTABLE_ARRAY(T, t, X)                                          \
  T*     new_##t(X* d, size_t n, bool s, ResizeAlgorithm ag);           \
  void   init_##t(T* a, X* _s, size_t ms, bool s, ResizeAlgorithm ag);  \
  void   free_##t(T* a);                                                \
  void   grow_##t(T* a, size_t n);                                      \
  void   shrink_##t(T* a, size_t n);                                    \
  size_t write_##t(T* a, X* s, size_t n);                               \
  size_t t##_push(T* a, X x);                                           \
  size_t t##_pushn(T* a, size_t n, ...);                                \
  size_t t##_pushv(T* a, size_t n, va_list va);                         \
  X      t##_pop(T* a);                                                 \
  X      t##_popn(T* a, size_t n, bool e)

MUTABLE_ARRAY(MVec, mvec, Val);
MUTABLE_ARRAY(Alist, alist, void*);

#undef MUTABLE_ARRAY

#endif
