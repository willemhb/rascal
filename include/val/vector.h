#ifndef rl_val_vector_h
#define rl_val_vector_h

#include "val/hamt.h"

/* Rascal Vector types.

   Vectors are Clojure-like immutable array tries.

   MutVecs are simple dynamic arrays of tagged values. */

/* C types */
struct VecNode {
  HEADER;
  union {
    void*     data;
    VecNode** children;
    Value*    slots;
  };
};

struct Vector {
  HEADER;
  Value*   tail;
  size_t   arity;
  VecNode* root;
};

struct MutVec {
  HEADER;
  Value* data;
  size_t cnt;
  size_t cap;
};

/* Globals */
extern Vector EmptyVec;
extern Type MutVecType, VectorType, VecNodeType;

/* External API */
// predicates & casts
#define is_mvec(x)     has_type(x, &MutVecType)
#define as_mvec(x)     as(MutVec*, untag48, x)

#define is_vec(x)      has_type(x, &VectorType)
#define as_vec(x)      as(Vector*, untag48, x)

#define is_vec_node(x) has_type(x, &VecNodeType)
#define as_vec_node(x) as(VecNode*, untag48, x)

/* Vector API */
// constructors
Vector* mk_vec(size_t n, Value* a);

// accessors & modifiers
Value   vec_ref(Vector* vec, size_t n);
Vector* vec_set(Vector* vec, size_t n, Value v);
Vector* vec_add(Vector* vec, Value v);
Vector* vec_del(Vector* vec);
Vector* vec_cat(Vector* vx, Vector* vy);

/* MutVec API */
MutVec*  new_mvec(void);
void     init_mvec(MutVec* arr);
void     free_mvec(MutVec* arr);
void     resize_mvec(MutVec* arr, size_t new_cnt);
size_t   mvec_push(MutVec* arr, Value x);
size_t   mvec_write(MutVec* arr, size_t n, Value* data);
size_t   mvec_pushn(MutVec* arr, size_t n, ...);
Value    mvec_pop(MutVec* arr);
Value    mvec_popn(MutVec* arr, size_t n);


#endif
