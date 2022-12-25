#include <stdlib.h>

#include "func.h"
#include "vec.h"
#include "code.h"
#include "sym.h"

#include "val.h"
#include "obj.h"
#include "type.h"
#include "memory.h"

#include "prin.h"

#include "tpl/impl/obj.h"

/* C types */
struct func_init_t {
  char *name;
  int   nargs;
  bool  vargs;

  union {
    opcode_t    label;
    native_fn_t funptr;
    struct module_t module;
  };
};

/* API */
/* external */
native_t make_native(char *name, int nargs, bool vargs, native_fn_t funptr) {
  struct func_init_t ini = {
    .name=name,
    .nargs=nargs,
    .vargs=vargs,
    .funptr=funptr
  };

  return (native_t)make_obj(native_obj, 0, &ini);
}

val_t native(char *name, int nargs, bool vargs, native_fn_t funptr) {
  native_t out = make_native(name, nargs, vargs, funptr);
  return tag_val(out, OBJECT);
}

val_t def_native(char *name, int nargs, bool vargs, native_fn_t funptr) {
  val_t symval  = native(name, nargs, vargs, funptr);
  
  return define(name, symval);
}

prim_t make_prim(char *name, int nargs, bool vargs, opcode_t label) {
  struct func_init_t ini = {
    .name=name,
    .nargs=nargs,
    .vargs=vargs,
    .label=label
  };

  return (prim_t)make_obj(prim_obj, 0, &ini);
}

val_t prim(char *name, int nargs, bool vargs, opcode_t label) {
  prim_t out = make_prim(name, nargs, vargs, label);

  return tag_val(out, OBJECT);
}

val_t def_prim(char *name, int nargs, bool vargs, opcode_t label) {
  val_t symval  = prim(name, nargs, vargs, label);

  return define(name, symval);
}

module_t make_module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts) {
  struct func_init_t ini = {
    .name=name,
    .nargs=nargs,
    .vargs=vargs,
    .module= { bcode, consts }
  };

  return (module_t)make_obj(module_obj, 0, &ini);
}

val_t module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts) {
  module_t out = make_module(name, nargs, vargs, bcode, consts);

  return tag_val(out, OBJECT);
}

val_t def_module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts) {
  val_t symval = module(name, nargs, vargs, bcode, consts);
}

/* internal */
void init_func(obj_t self, obj_type_t type, size_t n, void *ini) {
  (void)n;
  (void)type;

  if (ini) {
    struct func_init_t *fini = ini;

    func_name(self)  = fini->name;
    func_nargs(self) = fini->nargs;
    func_vargs(self) = fini->vargs;
  }
}

void init_prim(obj_t self, obj_type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  if (ini) {
    prim_label(self) = ((struct func_init_t*)ini)->label;
  }
}

void init_native(obj_t self, obj_type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  if (ini) {
    native_funptr(self) = ((struct func_init_t*)ini)->funptr;
  }
}

void init_module(obj_t self, obj_type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  if (ini) {
    module_bcode(self)  = ((struct func_init_t*)ini)->module.bcode ? : make_code(0, NULL);
    module_consts(self) = ((struct func_init_t*)ini)->module.consts ? : make_vec(0, NULL);
  }
}

void prin_func(val_t x) {
  printf("#func(%s %d)", func_name(x), func_nargs(x));
}

/* predicates */
bool val_is_func(val_t val) {
  rl_type_t type = rl_type(val);

  return type > sym_type && type < cons_type;
}

bool obj_is_func(obj_t obj) {
  obj_type_t type = obj_type(obj);

  return type > sym_obj && type < cons_obj;
}

gen_isa(native);
gen_isa(prim);
gen_isa(module);
gen_asa(native);
gen_asa(prim);
gen_asa(module);

/* initialization */
void func_init(void) {
  /* native initialization */
  Type[native_type] = (struct dtype_t) {
    .name="native",

    .prin=prin_func,

    .init=init_native,

    .head_size=sizeof(struct func_head_t),
    .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t),
    .body_size=sizeof(struct prim_t)
  };

  Type[prim_type] = (struct dtype_t) {
    .name="prim",

    .prin=prin_func,

    .init=init_prim,

    .head_size=sizeof(struct func_head_t),
    .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t),
    .body_size=sizeof(struct prim_t)
  };

  Type[module_type] = (struct dtype_t) {
    .name="module",

    .prin=prin_func,

    .init=init_prim,

    .head_size=sizeof(struct func_head_t),
    .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t),
    .body_size=sizeof(struct module_t)
  };
}
