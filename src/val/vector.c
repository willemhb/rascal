#include <string.h>

#include "util/number.h"

#include "vm/memory.h"

#include "val/seq.h"
#include "val/vector.h"

/* globals */
#define NODE_SIZE   64
#define LEVEL_SHIFT 6
#define MAX_LEVELS  8
#define LEVEL_MASK  0x3f

/* internal API */
static size_t tail_off(size_t arity) {
  if (arity < HAMT_LEVEL_SIZE)
    return 0;

  return ((arity - 1) >> HAMT_SHIFT) << HAMT_SHIFT;
}

static bool is_leaf_node(VecNode* node) {
  return get_hamt_shift(node) == 0;
}

static bool children_are_leaves(VecNode* node) {
  return get_hamt_shift(node) == HAMT_SHIFT;
}

static VecNode* last_vec_node_child(VecNode* node) {
  assert(!is_leaf_node(node));
  
  size_t cnt = get_hamt_cnt(node);

  return node->children[cnt-1];
}

static bool is_editp(void* obj) {
  return get_mfl(obj, EDITP);
}

static Value* array_for(Vector* vec, size_t i) {
  if (i >= tail_off(vec->arity))
    return vec->tail;

  VecNode* node = vec->root;
  size_t shift = get_hamt_shift(node);

  for (; shift > 0; shift -= HAMT_SHIFT) {
    size_t index = hamt_index_for_level(i, shift);
    node = node->children[index];
  }

  return node->data;
}

static bool space_in_tail(Vector* vec) {
  return get_hamt_cnt(vec) < HAMT_LEVEL_SIZE;
}

static bool space_in_node(VecNode* node) {
  if (is_leaf_node(node))
    return false;

  if (get_hamt_cnt(node) < HAMT_LEVEL_SIZE)
    return true;

  return space_in_node(last_vec_node_child(node));
}

static bool space_in_root(Vector* vec) {
  bool out = false;
  
  if (vec->root != NULL)
    out = space_in_node(vec->root);

  return out;
}

static void add_to_node(VecNode* node, Value* a) {
  size_t shift = get_hamt_shift(node);

  
}

static void add_to_tail(Vector* vec, Value v) {
  size_t cnt = get_hamt_cnt(vec);
  resize_hamt_array(vec, (void**)vec->tail, cnt+1, sizeof(Value));
  vec->tail[cnt] = v;
}

/* internal API */
// constructors
const size_t VecLeafHamtFl  = 0x00404000u;
const size_t VecLeafArrSize = HAMT_LEVEL_SIZE * sizeof(Value);

static void init_vec_leaf_data(VecLeaf* l, Value* a) {
  assert(a);
  memcpy(l->data, a, VecLeafArrSize);
}

static VecNode* mk_vec_leaf(Value* a) {
  VecLeaf* out = new_obj(&VecNodeType, VecLeafHamtFl, 0, 0);

  out->data = allocate(NULL, VecLeafArrSize);

  if (a)
    init_vec_leaf_data(out, a);

  return out;
}

static VecNode* mk_vec_node(Obj** data, bool editp, size_t cnt, size_t sh) {
  VecNode* out = new_obj(&VecNodeType, 0, editp*EDITP, 0);

  init_hamt(out, (void**)&out->data, data, cnt, sh, sizeof(Obj*));

  return out;
}

static Vector* new_vec(Value* a, VecNode* root, bool editp, size_t n, size_t sh) {
  Vector* out;

  if (root != NULL) {
    save(1, tag(root));
    out = new_obj(&VectorType, 0, editp*EDITP, 0);
  } else {
    out = new_obj(&VectorType, 0, editp*EDITP, 0);
  }

  out->root = root;
  init_hamt(out, (void**)&out->tail, a, n, sh, sizeof(Value));

  return out;
}

/* external API */

Vector* mk_vec(size_t n, Value* a) {
  Vector* out;
  
  if (n == 0)
    out = &EmptyVec;

  else if (n <= HAMT_LEVEL_SIZE) {
    out = new_vec(a, NULL, false, n, 0);
  } else {
    out = new_vec(NULL, NULL, true, 0, 0);

    for (size_t i=0; i<n; i++)
      vec_add(out, a[i]);

    freeze(out);
  }

  return out;
}

// accessors & modifiers
Value vec_ref(Vector* vec, size_t n) {
  assert(n < vec->arity);

  Value* vals = array_for(vec, n);

  return vals[n & LEVEL_MASK];
}

Vector* vec_add(Vector* vec, Value v) {
  if (is_editp(vec)) {
    if (space_in_tail(vec))
      add_to_tail(vec, v);

    else {
      save(2, tag(vec), v);

      if (space_in_root(vec)) {
        
      }
    }
  } else {
    save(2, tag(vec), v);
    vec = unfreeze(vec);
    vec = vec_add(vec, v);
    vec = freeze(vec);
  }

  return vec;
}

Vector* vec_set(Vector* vec, size_t n, Value v);

/* freeze/unfreeze */
Vector* freeze_vec(Vector* vec) {
  if (del_mfl(vec, EDITP)) {
    freeze_hamt_array(vec, (void**)&vec->tail, sizeof(Value));

    if (vec->root != NULL)
      vec->root = freeze(vec->root);
  }

  return vec;
}

Vector* unfreeze_vec(Vector* vec) {
  if (!is_editp(vec)) {
    vec = clone_obj(vec);
    set_mfl(vec, EDITP);
  }

  return vec;
}

VecNode* freeze_vec_node(VecNode* n) {
  if (del_mfl(n, EDITP)) {
    size_t shift = get_hamt_shift(n);

    if (shift > 0) {
      freeze_hamt_array(n, (void**)n->children, sizeof(VecNode*));
      size_t cnt = get_hamt_cnt(n);

      for (size_t i=0; i < cnt; i++) {
        if (is_editp(n->children[i]))
          freeze_vec_node(n->children[i]);
      }
    }
  }

  return n;
}

VecNode* unfreeze_vec_node(VecNode* n) {
  if (!is_editp(n)) {
    n = clone_obj(n);
    set_mfl(n, EDITP);
  }

  return n;
}
