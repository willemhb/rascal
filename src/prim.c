#include "prim.h"

#include "type.h"

/* C types */
typedef struct prim_init_t *prim_init_t;

struct prim_init_t {
  struct func_init_t func_init;
  opcode_t label;
};

/* globals */
void init_prim(obj_t self, type_t type, size_t n, void *ini);

struct type_t PrimType = {
  .name="prim",
  .prin=prin_func,
  .init=init_prim,
  .head_size=sizeof(struct func_head_t),
  .body_size=sizeof(struct prim_t),
  .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t)
};

/* API */
/* external */
prim_t make_prim(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, opcode_t label) {
  struct prim_init_t init = {
    .func_init = {
      .name=name,
      .nargs=nargs,
      .vargs=vargs,
      .guard=guard,
      .type=type
    },

    .label=label
  };

  return (prim_t)make_obj(&PrimType, 6, &init);
}

val_t prim(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, opcode_t label) {
  prim_t out = make_prim(name, nargs, vargs, guard, type, label);

  return tag_val(out, OBJECT);
}

/* internal */
void init_prim(obj_t self, type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  if (ini)
    as_prim(self)->label = ((prim_init_t)ini)->label;
}

/* native */
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

FALLBACK_NATIVE_CONSTRUCTOR(prim);

/* initialization */
void prim_init(void) {
  DEF_FALLBACK_NATIVE_CONSTRUCTOR(prim, PrimType);
}
