#ifndef rl_func_h
#define rl_func_h

#include "obj.h"
#include "sym.h"
#include "error.h"

/* C types */
struct func_t {
  char     *name;
  type_t    type;     // the type this function represents (if this function is a constructor)
  method_t  vmethod;  // at present only one variadic method is supported
  objs_t   *fmethods; // other methods indexed by arity
};

/* globals */
extern struct type_t FuncType;

/* API */
func_t     make_func(char *name, type_t type);
val_t      func(char *name, type_t type);
func_t     def_func(char *name, type_t type);
method_t   get_method(func_t f, int n);
method_t   add_method(func_t f, int n, bool v, guard_fn_t g, val_t h);
eval_err_t validate_func_call(func_t f, int n, val_t *a);

/* convenience */
#define is_func(x)   has_type(x, &FuncType)
#define as_func(x)   ((func_t)as_obj(x))

#define func_name(f)     (as_func(f)->name)
#define func_type(f)     (as_func(f)->type)
#define func_vmethod(f)  (as_func(f)->vmethod)
#define func_fmethods(f) (as_func(f)->fmethods)

#define is_type(f)       (func_type(f)!=NULL)
#define has_method(f, n) (get_method(as_func(f), n)!=NULL)

#endif
