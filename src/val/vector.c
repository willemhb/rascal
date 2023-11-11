#include <string.h>

#include "vm/memory.h"

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
  return arity & LEVEL_MASK ? : NODE_SIZE;
}

static bool tail_has_space(size_t arity) {
  return tail_size(arity) < NODE_SIZE;
}

static Value* array_for(Vector* vec, size_t i) {
  if (i >= tail_off(vec->arity))
    return vec->tail;

  Obj* node = (Obj*)vec->root;

  for (int level=vec->shift; level > 0; level -= LEVEL_SHIFT)
    node = ((VecNode*)node)->data[(i >> level) & LEVEL_MASK];

  return ((VecLeaf*)node)->data;
}

static void freeze_vec_leaf(VecLeaf* leaf) {
  if (leaf)
    del_obj_mfl((Obj*)leaf, EDITP);
}

static VecLeaf* unfreeze_vec_leaf(VecLeaf* leaf) {
  if (!get_obj_mfl((Obj*)leaf, EDITP)) {
    leaf = (VecLeaf*)clone_obj((Obj*)leaf);
    set_obj_mfl((Obj*)leaf, EDITP);
  }

  return leaf;
}

static VecNode* unfreeze_vec_node(VecNode* node) {
  if (!get_obj_mfl((Obj*)node, EDITP)) {
    node = (VecNode*)clone_obj((Obj*)node);
    set_obj_mfl((Obj*)node, EDITP);
  }

  return node;
}

static Vector* unfreeze_vec(Vector* vec) {
  if (!get_obj_mfl((Obj*)vec, EDITP)) {
    vec = (Vector*)clone_obj((Obj*)vec);
    set_obj_mfl((Obj*)vec, EDITP);
  }

  return vec;
}

static void freeze_vec_node(VecNode* node) {
  if (node && del_obj_mfl((Obj*)node, EDITP)) {
    for (size_t i=0; node->data[i] && i < NODE_SIZE; i++) {
      if (node->data[i]->type == &VecLeafType)
        freeze_vec_leaf((VecLeaf*)node->data[i]);
      else
        freeze_vec_node((VecNode*)node->data[i]);
    }
  }
}

static void freeze_vec(Vector* vec) {
  if (del_obj_mfl((Obj*)vec, EDITP))
    freeze_vec_node(vec->root);
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
VecLeaf* mk_vec_leaf(Value* a) {
  VecLeaf* out = (VecLeaf*)new_obj(&VecLeafType, 0, 0, 0);

  memcpy(out->data, a, NODE_SIZE * sizeof(Value));

  return out;
}

Vector* mk_vec(size_t n, Value* a) {
  if (n == 0)
    return &EmptyVec;

  else {
    Vector* out = (Vector*)new_obj(&VectorType, 0, EDITP, 0);

    out->root  = NULL;
    out->shift = 0;
    out->arity = 0;

    if (n <= NODE_SIZE) {
      memcpy(out->tail, a, n*sizeof(Value));
      out->arity = n;
    } else {
      save(1, tag_obj(out));
    
      for (size_t i=0; i<n; i++)
        vec_add(out, a[i]);
    }
    
    freeze_vec(out);

    return out;
  }
}

// accessors & modifiers
Value vec_ref(Vector* vec, size_t n) {
  assert(n < vec->arity);

  Value* vals = array_for(vec, n);

  return vals[n & LEVEL_MASK];
}

Vector* vec_add(Vector* vec, Value v) {
  if (get_obj_mfl((Obj*)vec, EDITP)) {
    
  } else {
    
  }
}

Vector* vec_set(Vector* vec, size_t n, Value v);
