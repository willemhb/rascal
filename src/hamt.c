#include <string.h>

#include "hamt.h"
#include "object.h"
#include "number.h"

#include "memory.h"

// misc -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define SHIFT      0x06ul
#define MAXN       0x40ul
#define MAXS       0x2aul
#define MINN       0x01ul
#define MASK       0x3ful
#define MAXH       0x07ul
#define MAXL       0x08ul

// internal type implementations ----------------------------------------------
// node -----------------------------------------------------------------------
void* alloc_node(data_type_t* type, usize count, flags fl);
void* copy_node(void* self, usize padding);
void  init_node(void* self, data_type_t* type, usize count, flags fl);
void  trace_node(void* self);
usize size_of_node(void* self);

data_type_t NodeType = {
  .type={
    .obj={
      .frozen=true,
      .type  =DATA_TYPE,
      .gray  =true
    },

    .name="node",
    .idno=NODE
  },

  .size =sizeof(node_t),

  .alloc  =alloc_node,
  .copy   =copy_node,
  .init   =init_node,
  .trace  =trace_node,
  
  .size_of=size_of_node
};

void* alloc_node(data_type_t* type, usize count, flags fl) {
  
}

void* copy_node(void* self, usize padding) {
  node_t* n = self, * out;

  if (padding) {
    usize newl = n->length + padding;
    usize newc = ceil2(newl);
    usize newp = (newc - n->capacity) * sizeof(void*);

    assert(newc <= MAXN);

    out = duplicate(self, rl_size_of(self), newp);
    out->capacity = newc;
  } else
    out = duplicate(n, rl_size_of(n), 0);

  return out;
}

void init_node(void* self, data_type_t* type, usize count, flags fl) {
  (void)type;
  (void)fl;

  node_t* n   = self;
  n->length   = count;
  n->capacity = ceil2(count);
}

void trace_node(void* self) {
  node_t* n = self;

  if (n->shift)
    trace_objects(n->length, (object_t**)n->node);

  else
    trace_values(n->length, n->leaf);
}

usize size_of_node(void* self) {
  assert(self);

  node_t* n = self;

  return n->capacity * sizeof(void*);
}

// stencil --------------------------------------------------------------------
void* alloc_stencil(data_type_t* type, usize n, flags fl);
void* copy_stencil(void* self, usize padding);
void  init_stencil(void* self, data_type_t* type, usize n, flags fl);
void  trace_stencil(void* self);
usize size_of_stencil(void* self);

data_type_t StencilType = {
  .type={
    .obj={
      .frozen=true,
      .type  =DATA_TYPE,
      .gray  =true
    },

    .name="stencil",
    .idno=STENCIL
  },

  .size =sizeof(stencil_t),

  .alloc  =alloc_stencil,
  .copy   =copy_stencil,
  .init   =init_stencil,
  .trace  =trace_stencil,
  
  .size_of=size_of_stencil
};

// utilities ------------------------------------------------------------------

// APIs -----------------------------------------------------------------------
// vector ---------------------------------------------------------------------

// dict -----------------------------------------------------------------------

#undef SHIFT
#undef MAXN
#undef MAXS
#undef MINN
#undef MASK
#undef MAXH
#undef MAXL
