#include <assert.h>
#include <stdarg.h>

#include "code.h"

#include "arr.h"

#include "memory.h"
#include "type.h"

#include "prin.h"

#include "def/opcodes.h"

#include "util/collection.h"

/* C types */

/* globals */

/* API */
/* external */
code_t make_code(size_t n, ushort *instr) {
  return (code_t)make_obj(code_obj, n, instr);
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
    i += code_len(code);

  assert(i >= 0 && (size_t)i < code_len(code));
  return code[i];
}

val_t code_set(code_t code, int i, val_t val) {
  if (i < 0)
    i += code_len(code);

  assert(i >= 0 && (size_t)i < code_len(code));
  code[i] = val;
  return val;
}

size_t code_write(code_t *code, ushort op, ...) {
  size_t argc = op_argc(op);
  size_t offset = code_len(*code);
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
  size_t l = code_len(c);

  for (size_t i=0; i < l; i++) {
    printf("%d", c[i]);
    
    if (i+1 < l)
      printf(" ");
  }

  printf(")");
}

/* convenience */
void dis_code(code_t code) {
  size_t offset = 0, end = code_len(code);

  while (offset < end)
    dis_op(code, &offset);
}

/* generics */
#include "tpl/impl/generic.h"

ISA_METHOD(code, val, rl_type, 1, code_type);
ISA_METHOD(code, obj, rl_type, 1, code_type);
ISA_NON0(code, code);
ASA_METHOD(code, val, is_code, as_obj);
ASA_METHOD(code, obj, is_code, NOOP_CNVT);
ASA_METHOD(code, code, NON0_GUARD, NOOP_CNVT);
HEAD_METHOD(code, val, is_code, as_obj);
HEAD_METHOD(code, obj, is_code, NOOP_CNVT);
HEAD_METHOD(code, code, NON0_GUARD, NOOP_CNVT);

/* initialization */
void code_init(void) {
  Type[code_type] = (struct dtype_t) {
    .name="code",

    /* general methods */
    .prin=prin_code,

    /* object model */
    .create=create_arr,
    .init=init_arr,
    .resize=resize_arr,
    .objsize=arr_size,
    .pad=pad_alist_size,

    /* layout */
    .head_size=sizeof(struct arr_head_t),
    .base_offset=sizeof(struct arr_head_t) - sizeof(struct obj_head_t),
    .body_size=0,
    .el_size=sizeof(ushort)
  };
}
