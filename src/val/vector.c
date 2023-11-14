#include <string.h>

#include "util/number.h"

#include "vm/memory.h"

#include "val/map.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/seq.h"
#include "val/vector.h"

/* Globals */
#define NODE_SIZE   64
#define LEVEL_SHIFT 6
#define MAX_LEVELS  8
#define LEVEL_MASK  0x3f

/* types */
/* Vector type */
extern void  trace_vec(void* obj);
extern void  finalize_vec(void* obj);
extern void* clone_vec(void* obj);

INIT_OBJECT_TYPE(Vector, NULL, trace_vec, finalize_vec, clone_vec);

/* VecNode type */
extern void  trace_vec_node(void* obj);
extern void  finalize_vec_node(void* obj);
extern void* clone_vec_node(void* obj);

INIT_OBJECT_TYPE(VecNode, NULL, trace_vec, finalize_vec, clone_vec);

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
static VecNode* mk_vec_node(Obj** children, size_t cnt, size_t sh) {
  VecNode* out = new_obj(&VecNodeType, 0, EDITP, 0);

  init_hamt(out, &out->children, );
}

static VecNode* fork_vec_node(size_t sh, VecLeaf* leaf) {
  VecNode* out;
  
  if (sh > HAMT_SHIFT) {
    VecNode* child = fork_vec_node(sh - HAMT_SHIFT, leaf);
    save(1, tag(child));
    out = mk_vec_node((Obj**)&child, 1, sh);
  } else {
    out = mk_vec_node((Obj**)&leaf, 1, sh);
  }

  return out;
}
