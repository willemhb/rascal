#ifndef rl_func_h
#define rl_func_h

#include "obj.h"
#include "sym.h"
#include "error.h"

/* C types */
struct func_t {
  char     *name;
  type_t    type;     // the type this function represents (if this function is a constructor)
  method_t  variadic; // at present only one variadic method is supported
  objs_t   *methods;  // other methods indexed by arity
};

/* globals */
extern struct type_t FuncType;

/* API */
func_t     make_func(char *name, type_t type);
bool       is_type(val_t self);
bool       has_method(func_t self, int n);
method_t   get_method(func_t func, int n);
method_t   add_method(func_t f, int n, bool v, guard_fn_t g, val_t h);

/* convenience */
#define is_func(x)   has_type(x, &FuncType)
#define as_func(x)   ((func_t)as_obj(x))

#endif
