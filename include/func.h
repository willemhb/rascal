#ifndef rl_func_h
#define rl_func_h

#include "obj.h"

#include "error.h"

/* C types */
typedef struct func_init_t *func_init_t;

struct func_init_t {
  char   *name;
  type_t  type;
};

struct func_t {
  sym_t     name;
  type_t    type;     // the type this function represents (if this function is a constructor)
  method_t  variadic; // at present only one variadic method is supported
  objs_t   *methods;  // other methods indexed by arity
};

/* globals */
extern struct type_t FuncType;

/* API */
func_t     make_func(sym_t name, type_t type);
bool       is_type(val_t self);
bool       has_method(func_t self);
method_t   get_method(func_t func, int n);
comp_err_t def_method(func_t func, int nargs, bool vargs, guard_fn_t guard, val_t handler);
comp_err_t bind_method(func_t func, int nargs, bool vargs, val_t handler);

/* convenience */
#define is_func(x)   has_type(x, &FuncType)
#define as_func(x)   ((func_t)as_obj(x))

#define def_func()

#define def_prim(name, nargs, vargs, guard, label)  \
  do {                                              \
    val_t __h__ = tag_val(label, SMALL);            \
    val_t __n__ = sym(name);                        \
    assert(is_bound(__n__));                        \
    val_t __f__ =                                   \
  } while (false)

#endif
