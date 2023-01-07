#ifndef rl_module_h
#define rl_module_h

#include "func.h"

/* C types */
typedef enum module_fl_t module_fl_t;

struct module_t {
  code_t bcode;
  vec_t  consts;
  vec_t  locals;
};

enum module_fl_t {
  toplevel_module=0x0000, // module created at the repl
  function_module=0x0001, // module created by the `fun` special form
};

/* globals */
extern struct type_t ModuleType;

/* API */
module_t make_module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts, vec_t locals);
val_t    module(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, code_t bcode, vec_t consts, vec_t locals);
val_t    get_module_const(module_t module, size_t n);
size_t   put_module_const(module_t module, val_t val);
int      get_module_local(module_t module, val_t name);
int      put_module_local(module_t module, val_t name);
size_t   code_size(module_t module);
size_t   emit_instr(module_t module, opcode_t op, ...);
void     fill_input(module_t module, size_t offset, ...);
void     dis_module(module_t module);

/* convenience */
#define is_module(x) has_type(x, &ModuleType)
#define as_module(x) ((module_t)as_obj(x))

#endif
