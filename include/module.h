#ifndef rl_module_h
#define rl_module_h

#include "func.h"

/* C types */
typedef enum module_type_t module_type_t;
typedef enum var_ref_t var_ref_t;

struct module_t {
  module_t parent;
  code_t   bcode;
  vec_t    consts;
  vec_t    locals;
};

enum module_type_t {
  toplevel_module=0x0000, // module created at the repl
  closure_module =0x0001, // module created by the `fun` special form
  handler_module =0x0002,
};

enum var_ref_t {
  var_ref_unbound=-1,
  var_ref_local,
  var_ref_nonlocal,
  var_ref_global
};

/* globals */
extern struct type_t ModuleType;

/* API */
module_t  make_module(char *name, int nargs, bool vargs, type_t type, code_t bcode, vec_t consts, vec_t locals, module_t parent);
val_t     module(char *name, int nargs, bool vargs, type_t type, code_t bcode, vec_t consts, vec_t locals, module_t parent);
val_t     get_module_const(module_t module, size_t n);
size_t    put_module_const(module_t module, val_t val);
var_ref_t get_module_ref(module_t module, val_t name, int *i, int *j);
var_ref_t put_module_ref(module_t module, val_t name, int *i, int *b);
size_t    code_size(module_t module);
size_t    emit_instr(module_t module, opcode_t op, ...);
void      fill_input(module_t module, size_t offset, ...);
size_t    emit_instrs(module_t module, size_t n, ...);
void      dis_module(module_t module);

/* convenience */
#define is_module(x) has_type(x, &ModuleType)
#define as_module(x) ((module_t)as_obj(x))

#endif
