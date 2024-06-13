#ifndef rl_val_array_h
#define rl_val_array_h

#include "val/object.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
// resize algorithm type

// vector root and node types
struct Vector {
  HEADER;

  // bit fields
  word_t shift  : 6;
  word_t trans  : 1;
  word_t packed : 1;

  // data fields
  size_t   count;
  VecNode* root;
  Value*   tail;
};

struct VecNode {
  HEADER;

  // bit fields
  word_t shift : 6;
  word_t trans : 1;

  uint32_t count, max_count;

  // data fields
  union {
    VecNode** children;
    Value     slots[0];
  };
};

// dynamic array types
#define DYNAMIC_ARRAY(X)                        \
  word_t algo   : 2;                            \
  word_t shrink : 1;                            \
  X* data;                                      \
  X* _static;                                   \
  size_t count, max_count, max_static

struct MutVec {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(Value);
};

struct Alist {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(void*);
};

#undef DYNAMIC_ARRAY

/* Globals */
// types
extern Type VectorType, VecNodeType, MutVecType, AlistType;

/* APIs */
/* Vector API */
#define as_vec(x) ((Vector*)as_obj(x))
#define is_vec(x) has_type(x, &VectorType)

Vector* mk_vec(size_t n, Value* d);
Vector* packed_vec(size_t n, Value* d);
Value   vec_ref(Vector* v, size_t n);
Vector* vec_add(Vector* v, Value x);
Vector* vec_set(Vector* v, size_t n, Value x);
Vector* vec_pop(Vector* v, Value* r);
Vector* vec_cat(Vector* x, Vector* y);

/* Dynamic array APIs */
#define MUTABLE_ARRAY(T, t, X)                                          \
  T*     new_##t(X* d, size_t n, bool p, bool s);                       \
  void   init_##t(T* a, X* _s, size_t _ss, bool i, bool p, bool s);     \
  void   free_##t(T* a);                                                \
  void   grow_##t(T* a, size_t n);                                      \
  void   shrink_##t(T* a, size_t n);                                    \
  size_t write_##t(T* a, X* s, size_t n);                               \
  size_t t##_push(T* a, X x);                                           \
  size_t t##_pushn(T* a, size_t n, ...);                                \
  size_t t##_pushv(T* a, size_t n, va_list va);                         \
  X      t##_pop(T* a);                                                 \
  X      t##_popn(T* a, size_t n, bool e)

MUTABLE_ARRAY(MutVec, mvec, Value);
MUTABLE_ARRAY(Alist, alist, void*);

#undef MUTABLE_ARRAY

#endif
