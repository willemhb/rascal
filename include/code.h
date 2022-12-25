#ifndef rl_code_h
#define rl_code_h

#include "obj.h"

/* C types */
struct code_head_t {
  size_t len, cap;
  obj_head_t obj;
};

/* API */
code_t make_code(size_t n, ushort *instr);
code_t code_from_obj(size_t n, val_t *instr);
val_t  code_ref(code_t code, int n);
val_t  code_set(code_t code, int n, val_t x);
size_t code_write(code_t *code, ushort op, ...);

/* convenience */
void dis_code(code_t code);

/* generics */
#include "tpl/decl/generic.h"
ISA_METHOD(code, val);
ISA_METHOD(code, obj);
ISA_METHOD(code, code);
ASA_METHOD(code, val);
ASA_METHOD(code, obj);
ASA_METHOD(code, code);
HEAD_METHOD(code, val);
HEAD_METHOD(code, obj);
HEAD_METHOD(code, code);

#define is_code(x)   GENERIC_CALL_3(is_code, code, obj, val, x)
#define as_code(x)   GENERIC_CALL_3(as_code, code, obj, val, x)
#define code_len(x)  GETF_3(len, code_head, code, obj, val, x)
#define code_cap(x)  GETF_3(len, code_head, code, obj, val, x)

#endif
