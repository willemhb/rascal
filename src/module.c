#include <stdarg.h>

#include "module.h"

#include "code.h"
#include "vec.h"

#include "type.h"

/* C types */
typedef struct module_init_t *module_init_t;

struct module_init_t {
  struct func_init_t func_init;

  code_t bcode;
  vec_t  consts;
};

/* globals */
void init_module(obj_t self, type_t type, size_t n, void *ini);

struct type_t ModuleType = {
  .name="module",
  .prin=prin_func,
  .init=init_module,
  .head_size=sizeof(struct func_head_t),
  .body_size=sizeof(struct module_t),
  .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t)
};

/* API */
/* external */
module_t make_module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts) {
  struct module_init_t init = {
    .func_init = {
      .name =name,
      .nargs=nargs,
      .vargs=vargs,
      .guard=guard,
      .type =type
    },

    .bcode =bcode,
    .consts=consts
  };

  return (module_t)make_obj(&ModuleType, 6, &init);
}

val_t module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts) {
  module_t out = make_module(name, nargs, vargs, guard, type, bcode, consts);

  return tag_val(out, OBJECT);
}

val_t get_module_const(module_t module, size_t n) {
  assert( n < vec_head(module->consts)->len);

  return vec_ref(module->consts, n);
}

size_t put_module_const(module_t module, val_t val) {
  vec_t consts = module->consts;
  size_t consts_len = vec_head(consts)->len, out;

  for (out=0; out<consts_len; out++) {
    if (val == consts[out])
      goto end;
  }

  vec_push(&module->consts, val);

 end:
  return out;
}

size_t emit_instr(module_t module, opcode_t op, ...) {
  va_list va; va_start(va, op);

  size_t out;

  switch (op_argc(op)) {
  case 0:  out = code_write(&module->bcode, op); break;
  case 1:  out = code_write(&module->bcode, op, va_arg(va, int)); break;
  case 2:  out = code_write(&module->bcode, op, va_arg(va, int), va_arg(va, int)); break;
  case 3:  out = code_write(&module->bcode, op, va_arg(va, int), va_arg(va, int), va_arg(va, int)); break;
  default: rl_unreachable();
  }

  va_end(va);

  return out;
}

void dis_module(module_t module) {
  code_t   code   = module->bcode;

  printf("disassembly of %s:\n", func_head(module)->name);
  dis_code(code);

}

/* internal */
void init_module(obj_t self, type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  code_t bcode = NULL;
  vec_t consts = NULL;

  if (ini) {
    bcode  = ((module_init_t)ini)->bcode;
    consts = ((module_init_t)ini)->consts;
  }

  bcode  = bcode  ? : make_code(0, NULL);
  consts = consts ? : make_vec(0, NULL);

  as_module(self)->bcode  = bcode;
  as_module(self)->consts = consts;
}
