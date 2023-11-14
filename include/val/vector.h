#ifndef rl_val_vector_h
#define rl_val_vector_h

#include "val/hamt.h"

/* Clojure-like immutable persistent ordered collection type. */

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

/* Globals */
extern Vector EmptyVec;
extern Type VectorType, VecNodeType, VecLeafType;

/* External API */
// predicates & casts
#define is_vec(x)      has_type(x, &VectorType)
#define as_vec(x)      as(Vector*, untag48, x)

#define is_vec_leaf(x) has_type(x, &VectorLeaf)
#define as_vec_leaf(x) as(VecLeaf*, untag48, x)

#define is_vec_node(x) has_type(x, &VecNodeType)
#define as_vec_node(x) as(VecNode*, untag48, x)

// miscellaneous helpers
// constructors
Vector* mk_vec(size_t n, Value* a);

// accessors & modifiers
Value   vec_ref(Vector* vec, size_t n);
Vector* vec_set(Vector* vec, size_t n, Value v);
Vector* vec_add(Vector* vec, Value v);
Vector* vec_del(Vector* vec);

#endif
