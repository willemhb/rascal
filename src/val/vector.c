#include "val/vector.h"

/* globals */
#define NODE_SIZE   64
#define LEVEL_SHIFT 6
#define MAX_LEVELS  8
#define LEVEL_MASK  0x3f

/* internal API */
static size_t tail_off(size_t arity) {
  if (arity < NODE_SIZE)
    return 0;

  return ((arity - 1) >> LEVEL_SHIFT) << LEVEL_SHIFT;
}

static size_t tail_size(size_t arity) {
  
}

static Value* array_for(Vector* vec, size_t i) {
  if (i >= tail_off(vec->arity))
    return vec->tail;

  Obj* node = (Obj*)vec->root;

  for (int level=vec->shift; level > 0; level -= LEVEL_SHIFT)
    node = ((VecNode*)node)->data[(i >> level) & LEVEL_MASK];

  return ((VecLeaf*)node)->data;
}

/* External API */
// predicates & casts
bool val_is_vec(Value x) {
  return type_of(x) == &VectorType;
}

bool obj_is_vec(Obj* obj) {
  return obj->type == &VectorType;
}

Vector* as_vec(Value x) {
  return (Vector*)untag_48(x);
}

// constructors
Vector* new_vec(size_t n) {
  assert(n > 0);

  
}

Vector* mk_vec(size_t n, Value* a) {
  if (n == 0)
    return &EmptyVec;

  else if (n <= NODE_SIZE) {
    
  }
}

// accessors & modifiers
Value vec_ref(Vector* vec, size_t n) {
  assert(n < vec->arity);

  Value* vals = array_for(vec, n);

  return vals[];
}

Vector* vec_set(Vector* vec, size_t n, Value v);
Vector* vec_add(Vector* vec, Value v);

