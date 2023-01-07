#include <stdarg.h>

#include "module.h"

#include "code.h"
#include "vec.h"
#include "sym.h"

#include "type.h"

#include "prin.h"

#include "util/ios.h"

/* C types */
typedef struct module_init_t *module_init_t;

struct module_init_t {
  struct func_init_t func_init;

  code_t bcode;
  vec_t  consts;
  vec_t  locals;
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
module_t make_module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts, vec_t locals)  {
  name = as_sym(sym(name));

  struct module_init_t init = {
    .func_init = {
      .name =name,
      .nargs=nargs,
      .vargs=vargs,
      .guard=guard,
      .type =type
    },

    .bcode =bcode,
    .consts=consts,
    .locals=locals
  };

  return (module_t)make_obj(&ModuleType, 6, &init);
}

val_t module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts, vec_t locals) {
  module_t out = make_module(name, nargs, vargs, guard, type, bcode, consts, locals);

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

int get_module_local(module_t module, val_t name) {
  vec_t  locals     = module->locals;
  size_t num_locals = vec_head(locals)->len;

  for (size_t i=0; i<num_locals; i++) {
    if (name == locals[i])
      return i;
  }

  return -1;
}

int put_module_local(module_t module, val_t name) {
  vec_t locals      = module->locals;
  size_t num_locals = vec_head(locals)->len, out;

  for (out=0; out<num_locals; out++) {
    if (name == locals[name])
      goto end;
  }

  vec_push(&module->locals, name);

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

size_t code_size(module_t module) {
  return code_head(module->bcode)->len;
}

void fill_input(module_t module, size_t offset, ...) {
  va_list va; va_start(va, offset);

  opcode_t op = code_ref(module->bcode, offset);

  switch (op_argc(op)) {
  case 1:
    code_set(module->bcode, offset+1, va_arg(va, int));
    break;

  case 2:
    code_set(module->bcode, offset+1, va_arg(va, int));
    code_set(module->bcode, offset+2, va_arg(va, int));
    break;

  case 3:
    code_set(module->bcode, offset+1, va_arg(va, int));
    code_set(module->bcode, offset+2, va_arg(va, int));
    code_set(module->bcode, offset+3, va_arg(va, int));
    break;
  }

  va_end(va);
}

void dis_module(module_t module) {
  code_t code   = module->bcode;
  vec_t  consts = module->consts;
  vec_t  locals = module->locals;

  printf("disassembly of %s:\n\n", func_head(module)->name);
  printf("    constants: "); prin(tag_val(consts, OBJECT)); newline();
  printf("    locals: "); prin(tag_val(locals, OBJECT)); newline();
  printf("    bytecode:  "); prin(tag_val(code, OBJECT)); newline(); newline();
  dis_code(code); newline();
}

/* internal */
void init_module(obj_t self, type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  code_t bcode = NULL;
  vec_t consts = NULL;
  vec_t locals = NULL;

  if (ini) {
    bcode  = ((module_init_t)ini)->bcode;
    consts = ((module_init_t)ini)->consts;
    locals = ((module_init_t)ini)->locals;
  }

  bcode  = bcode  ? : make_code(0, NULL);
  consts = consts ? : make_vec(0, NULL);
  locals = locals ? : make_vec(0, NULL);

  as_module(self)->bcode  = bcode;
  as_module(self)->consts = consts;
  as_module(self)->locals = locals;
}

/* native functions */
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

func_err_t guard_module(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(sym, args, 0);
  TYPE_GUARD(code, args, 1);
  TYPE_GUARD(vec, args, 2);
  TYPE_GUARD(vec, args, 3);

  return func_no_err;
}

val_t native_module(size_t nargs, val_t *args) {
  (void)nargs;

  sym_t  name   = as_sym(args[0]);
  code_t bcode  = as_code(args[1]);
  vec_t  consts = as_vec(args[2]);
  vec_t  locals = as_vec(args[3]);

  return module(name, 0, false, NULL, NULL, bcode, consts, locals);
}

/* initialization */
void module_init(void) {
  def_native("module", 4, false, guard_module, &ModuleType, native_module);
}
