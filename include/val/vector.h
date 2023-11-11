#ifndef rl_val_vector_h
#define rl_val_vector_h

#include "val/object.h"

/* Clojure-like immutable persistent ordered collection type. */

/* C types */
typedef struct VecNode VecNode;
typedef struct VecLeaf VecLeaf;

struct VecNode {
  HEADER;
  Obj*  data[64];
};

struct VecLeaf {
  HEADER;
  Value  data[64];
};

struct Vector {
  HEADER;
  VecNode* root;
  size_t   arity;
  Value    tail[];
};

/* Globals */
extern Vector EmptyVec;
extern Type VectorType, VecNodeType, VecLeafType;

/* External API */
// predicates & casts
bool    val_is_vec(Value x);
bool    obj_is_vec(Obj* obj);
Vector* as_vec(Value x);
size_t  vec_shift(Vector* vec);
size_t  vec_cnt(Vector* vec);

// constructors
Vector* mk_vec(size_t n, Value* a);

// accessors & modifiers
Value   vec_ref(Vector* vec, size_t n);
Vector* vec_set(Vector* vec, size_t n, Value v);
Vector* vec_add(Vector* vec, Value v);

#endif
