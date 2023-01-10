#ifndef rl_module_h
#define rl_module_h

#include "obj.h"

/* C types */
typedef enum module_type_t module_type_t;
typedef enum var_ref_t var_ref_t;

struct module_t {
  char    *name;
  code_t   bcode;
  vec_t    consts;
  vec_t    names;
  vec_t    upvals;
};

/* globals */
extern struct type_t ModuleType;

/* API */
module_t  make_module(char *n, code_t b, vec_t c, objs_t *u);
val_t     module(char *n, code_t b, vec_t c, objs_t *u);

val_t     module_constant(module_t m, int n);
val_t     module_upvalue(module_t m, int n, vals_t *e);
ushort    module_instr(module_t m, int n);
void      dis_module(module_t m);

/* convenience */
#define is_module(x) has_type(x, &ModuleType)
#define as_module(x) ((module_t)as_obj(x))

#endif
