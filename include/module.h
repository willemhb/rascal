#ifndef rl_module_h
#define rl_module_h

#include "func.h"

/* C types */
struct module_t {
  code_t bcode;
  vec_t  consts;
};

/* globals */
extern struct type_t ModuleType;

/* API */
module_t make_module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts);
val_t    module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts);
val_t    get_module_const(module_t module, size_t n);
size_t   put_module_const(module_t module, val_t val);
size_t   emit_instr(module_t module, opcode_t op, ...);
void     finalize_module(module_t module);

/* initialization */
void module_init(void);

#endif
