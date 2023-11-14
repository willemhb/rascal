#include <stdarg.h>
#include <string.h>

#include "util/number.h"

#include "vm/memory.h"

#include "val/table.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/seq.h"
#include "val/vector.h"

/* Implementation for Vector & MutVec types. */

/* Globals */
#define NODE_SIZE   64
#define LEVEL_SHIFT 6
#define MAX_LEVELS  8
#define LEVEL_MASK  0x3f

/* types */
/* MutVec type */
extern void  trace_mvec(void* obj);
extern void  finalize_mvec(void* obj);
extern void* clone_mvec(void* obj);

INIT_OBJECT_TYPE(MutVec,
                 .tracefn=trace_mvec,
                 .finalizefn=finalize_mvec,
                 .clonefn=clone_mvec);

/* Vector type */
extern void   trace_vec(void* obj);
extern void   finalize_vec(void* obj);
extern void*  clone_vec(void* obj);
extern hash_t hash_vec(Value x);
extern bool   equal_vecs(Value x, Value y);
extern int    order_vecs(Value x, Value y);

INIT_OBJECT_TYPE(Vector,
                 .tracefn=trace_vec,
                 .finalizefn=finalize_vec,
                 .clonefn=clone_vec,
                 .hashfn=hash_vec,
                 .egalfn=equal_vecs,
                 .ordfn=order_vecs);

/* VecNode type */
extern void   trace_vec_node(void* obj);
extern void   finalize_vec_node(void* obj);
extern void*  clone_vec_node(void* obj);

INIT_OBJECT_TYPE(VecNode,
                 .tracefn=trace_vec_node,
                 .finalizefn=finalize_vec_node,
                 .clonefn=clone_vec_node);

/* global singletons */
Vector EmptyVec = {
  .obj={
    .type =&VectorType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOFREE|NOTRACE|GRAY,
  },
  .tail =NULL,
  .arity=0,
  .root =NULL,
};

/* don't calculate this every time */
const size_t VecLeafHamtFl  = 0x00404000u;
const size_t VecLeafArrSize = HAMT_LEVEL_SIZE * sizeof(Value);

/* internal API */
static size_t tailoff(Vector* vec) {
  if (vec->arity <= HAMT_LEVEL_SIZE)
    return 0;

  return ((vec->arity - 1) >> HAMT_SHIFT) << HAMT_SHIFT;
}

static Value* array_for(Vector* vec, size_t n) {
  if (n >= tailoff(vec))
    return vec->tail;

  VecNode* node = vec->root;

  for (size_t shift=get_hamt_shift(node); shift > 0; shift -= HAMT_SHIFT)
    node = node->children[n >> shift & HAMT_LEVEL_MASK];

  return node->slots;
}

static bool is_leaf_node(VecNode* node) {
  return get_hamt_shift(node) == 0;
}

static bool space_in_node(VecNode* node) {
  bool out;
  
  if (node == NULL)
    out = false;

  else if (get_hamt_shift(node) == 0)
    out = false;

  else if (get_hamt_cnt(node) < HAMT_LEVEL_SIZE)
    out = true;

  else
    out = space_in_node(node->children[HAMT_LEVEL_SIZE-1]);

  return out;
}

static bool space_in_tail(Vector* vec) {
  return get_hamt_cnt(vec) < HAMT_LEVEL_SIZE;
}

static VecNode* vec_node_set(VecNode* node, size_t n, Value v) {
  if (!is_editp(node)) {
    node = unfreeze(node);
    save(1, tag(node));
    node = vec_node_set(node, n, v);
    del_mfl(node, EDITP);
  } else {
    size_t shift = get_hamt_shift(node);

    if (shift == 0)
      node->slots[n & HAMT_LEVEL_MASK] = v;

    else {
      size_t child_index = n >> shift && HAMT_LEVEL_MASK;
      VecNode* child = node->children[child_index];
      child = vec_node_set(child, n, v);
      node->children[child_index] = child;
    }
  }

  return node;
}

static VecNode* mk_vec_node(Obj** children, bool editp, size_t cnt, size_t sh) {
  VecNode* out = new_obj(&VecNodeType, 0, editp*EDITP, 0);
  init_hamt(out, &out->children, children, cnt, sh);
  return out;
}

static VecLeaf* mk_vec_leaf(Value* slots) {
  VecLeaf* out = new_obj(&VecNodeType, VecLeafHamtFl, 0, 0);
  out->slots = allocate(NULL, VecLeafArrSize);
  memcpy(out->slots, slots, VecLeafArrSize);

  return out;
}

static Vector* new_vec(void) {
  Vector* out = new_obj(&VectorType, 0, EDITP, 0);
  init_hamt(out, &out->tail, NULL, 0, 0);
  out->arity = 0;
  out->root  = NULL;

  return out;
}

static VecNode* fork_vec_node(bool editp, size_t sh, VecLeaf* leaf) {
  VecNode* out;

  if (sh > HAMT_SHIFT) {
    VecNode* child = fork_vec_node(editp, sh - HAMT_SHIFT, leaf);
    save(1, tag(child));
    out = mk_vec_node((Obj**)&child, editp, 1, sh);
  } else {
    out = mk_vec_node((Obj**)&leaf, editp, 1, sh);
  }

  return out;
}

static void write_tail(Vector* vec, Value* a, size_t n) {
  assert(n < HAMT_LEVEL_SIZE);

  resize_hamt_array(vec, &vec->tail, n);
  memcpy(vec->tail, a, n * sizeof(Value));
}

static void add_to_tail(Vector* vec, Value x) {
  hamt_push(vec, &vec->tail, (void*)x);
}

static VecNode* add_leaf(VecNode* node, bool editp, VecLeaf* leaf) {
  VecNode* out;

  save(1, tag(node));

  if (!is_editp(node)) {
    out = unfreeze(node);
    out = add_leaf(node, editp, leaf);

    if (editp)
      out = freeze(out);
  } else {
    size_t shift = get_hamt_shift(node);
    size_t cnt = get_hamt_cnt(node);
    VecNode* child;

    if (space_in_node(node)) {
      out = node;
      
      if (shift == HAMT_SHIFT)
        hamt_push(node, &node->children, leaf);
      
      else if (space_in_node(node->children[cnt-1])) {
        child = node->children[cnt-1];
        node->children[cnt-1] = add_leaf(child, editp, leaf);
      }

      else {
        child = fork_vec_node(editp, shift - HAMT_SHIFT, leaf);
        hamt_push(node, &node->children, child);
      }
    } else {
      out = mk_vec_node((Obj**)&node, editp, 1, shift + HAMT_SHIFT);
      save(1, tag(out));
      child = fork_vec_node(editp, shift, leaf);
      hamt_push(out, &out->children, child);
    }
  }

  return out;
}

static VecNode* last_child(VecNode* node) {
  size_t cnt = get_hamt_cnt(node);

  return node->children[cnt-1];
}

static VecLeaf* last_leaf(VecNode* node) {
  while (get_hamt_shift(node) > 0)
    node = last_child(node);
  return node;
}

static bool is_singleton_node(VecNode* node) {
  assert(node != NULL);

  if (is_leaf_node(node))
    return false;

  else if (get_hamt_cnt(node) > 1)
    return false;

  else if (get_hamt_shift(node) == HAMT_SHIFT)
    return true;

  else
    return is_singleton_node(node->children[0]);
}

static VecNode* pop_leaf_from_node(VecNode* node) {
  VecNode* out = node;
  
  if (is_leaf_node(node)) {
    out = NULL;
  } else if (is_singleton_node(node)) {
    out = NULL;
  } else if (!is_editp(node)) {
    out = node = unfreeze(node);
    save(1, tag(node));
    node = pop_leaf_from_node(node);
    node = freeze(node);
  } else {
    size_t cnt   = get_hamt_cnt(node);
    VecNode* child = pop_leaf_from_node(node->children[cnt-1]);

    if (child == NULL)
      resize_hamt_array(node, &node->children, cnt-1);

    else
      node->children[cnt-1] = child;
  }

  return out;
}

static VecLeaf* pop_leaf(Vector* vec) {
  VecLeaf* out = last_leaf(vec->root);
  VecNode* new_root = vec->root;
  size_t arity = vec->arity;
  size_t shift = get_hamt_shift(vec);
  size_t new_shift = shift;
  size_t new_arity = arity - 1;
  size_t new_root_arity = new_arity < HAMT_LEVEL_SIZE ? 0 : new_arity - HAMT_LEVEL_SIZE;

  /* Check for special cases. */
  /* If the root has only one leaf, don't reallocate anything, just set the
     root to NULL and return its only child. */
  
  if (new_root_arity == 0) {
    new_root = NULL;
    new_shift = 0;
  }

  /* If the root has just two children and one of them is full, we can share it
     entirely without reallocating anything. */
  else if (ispow64(new_root_arity)) {
    new_root = vec->root->children[0];
    new_shift = shift - HAMT_SHIFT;
  } else {
    new_root = pop_leaf_from_node(new_root);
  }

  vec->root = new_root;
  set_hamt_shift(vec, new_shift);
  return out;
}

static void push_tail(Vector* vec, bool editp, Value* a) {
  save(2, tag(vec), NUL);

  VecLeaf* l = mk_vec_leaf(a);

  add_saved(1, tag(l));

  if (vec->root == NULL) {
    vec->root = mk_vec_node((Obj**)&l, editp, 1, HAMT_SHIFT);
  } else {
    vec->root = add_leaf(vec->root, editp, l);
  }

  set_hamt_shift(vec, get_hamt_shift(vec->root));
}

/* External API */
/* Vector API */
Vector* mk_vec(size_t n, Value* a) {
  assert(n <= MAX_ARITY);
  Vector* out;

  if (n == 0)
    out = &EmptyVec;

  else {
    out = new_vec();
    size_t arity = n;

    for (; n > HAMT_LEVEL_SIZE; n -= HAMT_LEVEL_SIZE, a += HAMT_LEVEL_SIZE)
      push_tail(out, true, a);

    write_tail(out, a, n);
    out->arity = arity;
    freeze(out);
  }

  return out;
}

Value vec_ref(Vector* vec, size_t n) {
  assert(n < vec->arity);

  Value* slots = array_for(vec, n);
  return slots[n & HAMT_LEVEL_MASK];
}

Vector* vec_set(Vector* vec, size_t n, Value v) {
  assert(n < vec->arity);

  if (!is_editp(vec)) {
    save(2, tag(vec), v);
    vec = unfreeze(vec);
    vec = vec_set(vec, n, v);
    vec = freeze(vec);
  } else if (n >= tailoff(vec))
    vec->tail[n & HAMT_LEVEL_MASK] = v;
  else {
    vec->root = vec_node_set(vec->root, n, v);
  }

  return vec;
}

Vector* vec_add_(Vector* vec, Value v, bool editp) {
  assert(vec->arity < MAX_ARITY);

  if (!is_editp(vec)) {
    vec = unfreeze(vec);
    vec = vec_add_(vec, v, editp);

    if (!editp)
      vec = freeze(vec);

  } else {
    if (!space_in_tail(vec)) {
      push_tail(vec, editp, vec->tail);
      resize_hamt_array(vec, &vec->tail, 0);
    }

    add_to_tail(vec, v);
  }

  return vec;
}

Vector* vec_add(Vector* vec, Value v) {
  return vec_add_(vec, false, v);
}

Vector* vec_del(Vector* vec) {
  assert(vec->arity > 0);

  if (vec->arity == 1)
    vec = &EmptyVec;

  else if (!is_editp(vec)) {
    save(1, tag(vec));
    vec = unfreeze(vec);
    vec = vec_del(vec);
    vec = freeze(vec);
  } else {
    size_t new_size = get_hamt_cnt(vec) - 1;
    resize_hamt_array(vec, &vec->tail, new_size);

    if (new_size == 0) {
      VecLeaf* l = pop_leaf(vec);
      write_tail(vec, l->slots, HAMT_LEVEL_SIZE);
    }

    vec->arity -= 1;
  }

  return vec;
}

Vector* vec_cat(Vector* vx, Vector* vy) {
  Vector* out;
  
  if (vx->arity == 0)
    out = vy;

  else if (vy->arity == 0)
    out = vx;

  else {
    save(2, tag(vx), tag(vy));
    out = unfreeze(vx);
    add_saved(0, tag(out));

    for (size_t i=0; i<vy->arity; i++) {
      Value x = vec_ref(vy, i);
      vec_add_(out, x, true);
    }

    out = freeze(out);
  }

  return out;
}

/* MutVec API */
MutVec* new_mvec(void) {
  MutVec* out = new_obj(&MutVecType, 0, EDITP, 0);
  init_mvec(out);
  return out;
}

void init_mvec(MutVec* arr) {
  arr->data = NULL;
  arr->cnt  = 0;
  arr->cap  = 0;
}

void free_mvec(MutVec* arr) {
  deallocate(NULL, arr->data, 0);
  init_mvec(arr);
}

void resize_mvec(MutVec* arr, size_t new_cnt) {
  if (new_cnt == 0)
    free_mvec(arr);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = arr->cnt;
    old_cap = arr->cap;
    new_cap = pad_alist_size(old_cnt, new_cnt, old_cap, false);

    if (new_cap != old_cap) {
      arr->data = reallocate(NULL,
                             arr->data,
                             old_cap*sizeof(Value),
                             new_cap*sizeof(Value));
      arr->cap  = new_cap;
    }

    arr->cnt = new_cnt;
  }
}

size_t mvec_push(MutVec* arr, Value x) {
  size_t out;

  out = arr->cnt;
  resize_mvec(arr, out+1);
  arr->data[out] = x;

  return out;
}

size_t mvec_write(MutVec* arr, size_t n, Value* data) {
  size_t out = arr->cnt;

  resize_mvec(arr, out+n);

  if (data != NULL)
    memcpy(arr->data+out, data, n * sizeof(Value));

  return out;
}

size_t mvec_pushn(MutVec* arr, size_t n, ...) {
  Value buf[n];
  va_list va;

  va_start(va, n);

  for (size_t i=0; i<n; i++)
    buf[i] = va_arg(va, Value);

  va_end(va);

  return mvec_write(arr, n, buf);
}

Value mvec_pop(MutVec* arr) {
  Value out;
  
  assert(arr->cnt > 0);
  out = arr->data[arr->cnt-1];
  resize_mvec(arr, arr->cnt-1);

  return out;
}

Value mvec_popn(MutVec* arr, size_t n) {
  Value out;

  assert(n <= arr->cnt);
  out = arr->data[arr->cnt-n];
  resize_mvec(arr, arr->cnt-n);

  return out;
}
