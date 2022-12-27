#include <assert.h>
#include <stdarg.h>

#include "code.h"

#include "real.h"

#include "memory.h"
#include "type.h"

#include "prin.h"

#include "def/opcodes.h"

#include "util/collection.h"

/* C types */

/* globals */
void prin_code(val_t val);

struct type_t CodeType = {
  .name="code",
  .prin=prin_code,
  .create=create_arr,
  .init=init_arr,
  .resize=resize_arr,
  .pad=pad_stack_size,
  .head_size=sizeof(struct code_head_t),
  .base_offset=sizeof(struct code_head_t) - sizeof(struct obj_head_t),
  .el_size=sizeof(ushort)
};

/* API */
/* external */
code_t make_code(size_t n, ushort *instr) {
  return (code_t)make_obj(&CodeType, n, instr);
}

code_t code_from_obj(size_t n, val_t *instrs) {
  ushort buf[n];

  for (size_t i=0; i<n; i++) {
    val_t instr = instrs[i];

    assert(is_real(instr));

    buf[i] = (ushort)as_real(instr);
  }

  return make_code(n, buf);
}

val_t code_ref(code_t code, int i) {
  if (i < 0)
    i += code_head(code)->len;

  assert_bound(i, code_head(code)->len);
  return code[i];
}

val_t code_set(code_t code, int i, val_t val) {
  if (i < 0)
    i += code_head(code)->len;

  assert_bound(i, code_head(code)->len);
  code[i] = val;
  return val;
}

size_t code_write(code_t *code, ushort op, ...) {
  size_t argc = op_argc(op);
  size_t offset = code_head(*code)->len;
  size_t out = offset+argc+1;

  *(obj_t*)code = resize_obj(*(obj_t*)code, out);

  (*code)[offset] = op;
  va_list va; va_start(va, op);

  if (argc > 0)
    (*code)[offset+1] = va_arg(va, int);

  if (argc > 1)
    (*code)[offset+2] = va_arg(va, int);

  if (argc > 2)
    (*code)[offset+3] = va_arg(va, int);

  va_end(va);

  return out;
}

/* internal */
void prin_code(val_t x) {
  printf("#code(");

  code_t  c = as_code(x);
  size_t l = code_head(c)->len;

  for (size_t i=0; i < l; i++) {
    printf("%d", c[i]);
    
    if (i+1 < l)
      printf(" ");
  }

  printf(")");
}

/* convenience */
void dis_code(code_t code) {
  size_t offset = 0, end = code_head(code)->len;

  while (offset < end)
    dis_op(code, &offset);
}

/* natives */
#include "func.h"
#include "native.h"
#include "sym.h"
#include "tpl/impl/funcall.h"

func_err_t code_method_guard(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(code, args, 0);

  return func_no_err;
}

func_err_t code_accessor_guard(size_t nargs, val_t *args) {
  TYPE_GUARD(code, args, 0);
  INDEX_GUARD(code_head(args[0])->len, args, 1);
  for (size_t i=2; i<nargs; i++) {
    TYPE_GUARD(small, args, i);
  }
  return func_no_err;
}

func_err_t code_constructor_guard(size_t nargs, val_t *args) {
  for (size_t i=0; i<nargs; i++) {
    TYPE_GUARD(small, args, i);
  }

  return func_no_err;
}

val_t native_code(size_t nargs, val_t *args) {
  ushort buf[nargs];

  for (size_t i=0; i<nargs; i++)
    buf[i] = as_small(args[i]);

  code_t out = make_code(nargs, buf);

  return tag_val(out, OBJECT);
}

void code_init(void) {
  val_t constructor = native("code", 0, true, code_constructor_guard, &CodeType, native_code);

  define("code", constructor);
}
