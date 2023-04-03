#include <string.h>

#include "hamt.h"
#include "metaobject.h"
#include "object.h"
#include "number.h"

#include "memory.h"

// misc -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define NODE_SHIFT 6
#define NODE_MAXN 64u
#define NODE_MINN 1u
#define NODE_MASK 0x3ful
#define NODE_MAXH 7ul
#define NODE_MAXL 8ul

// APIs -----------------------------------------------------------------------
// vector ---------------------------------------------------------------------
// globals --------------------------------------------------------------------
vector_t EmptyVector = {
  .obj={
    .frozen=true,
    .gray  =true,
    .type  =VECTOR
  }
};

// local helpers --------------------------------------------------------------
static vecnode_t* vecnode(usize length, object_t** children) {
  vecnode_t* out = allocate(sizeof(vecnode_t));
  init_object(out, &VecNodeType);

  out->length = length;

  if (children)
    memcpy(out->children, children, length * sizeof(object_t*));

  return out;
}

static vecleaf_t* vecleaf(value_t values[64]) {
  vecleaf_t* out = allocate(sizeof(vecleaf_t));
  init_object(out, &VecLeafType);
  memcpy(out->values, values, 64 * sizeof(value_t));
  return out;
}

static usize tailoff(vector_t* self) {
  if (self->arity <= NODE_MAXN)
    return 0;

  return self->arity & ~NODE_MASK;
}

static value_t* array_for(vector_t* self, usize n) {
  if (n >= tailoff(self))
    return self->tail;

  object_t* node = (object_t*)self->root;

  for (usize off=self->offset; off > 0; off -= NODE_SHIFT)
    node = (object_t*)((vecnode_t*)node)->children[n >> off & NODE_MASK];

  return ((vecleaf_t*)node)->values;
}

static void* editable(void* node) {
  assert(node);

  object_t* obj = node;

  if (is_frozen(obj)) {
    obj = duplicate(obj, rl_size_of(obj));
    unfreeze(obj);
  }

  return obj;
}

static int append_tail(object_t** buffer, uint offset, value_t tail[64]) {
  
}

value_t vector_get(vector_t* self, usize n) {
  assert(n < self->arity);

  value_t* arr = array_for(self, n);

  return arr[n & NODE_MASK];
}

vector_t* vector_set(vector_t* self, usize n, value_t x) {
  vector_t* out = editable(self);
  bool do_freeze = out != self;

  if (n >= tailoff(out)) {
    out->tail[n & NODE_MASK] = x;

  } else {
    object_t* node = (object_t*)self->root, ** buf = (object_t**)&self->root, * pnode = (object_t*)out;

    for (usize off = self->offset; off > 0; off -= NODE_SHIFT) {
      node = *buf = editable(node);
      pnode->frozen = do_freeze;
      buf = &((vecnode_t*)node)->children[n >> off & NODE_MASK];
      node = *buf;
    }

    node = *buf = editable(node);
    pnode->frozen = do_freeze;
    ((vecleaf_t*)node)->values[n & NODE_MASK] = x;
    node->frozen = do_freeze;
  }

  return out;
}

vector_t* vector_add(vector_t* self, value_t x) {
  vector_t* out = editable(self);
  bool do_freeze = out != self;

  if (out->length == NODE_MAXN) {
    out->offset  = append_tail((object_t**)&out->root, out->offset, out->tail);
    out->length = 0;
  }

  out->tail[out->length++] = x;
  out->arity++;
  header(out)->frozen = do_freeze;

  return out;
}
