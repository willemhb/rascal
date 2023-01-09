#ifndef rl_func_h
#define rl_func_h

#include "obj.h"
#include "sym.h"
#include "error.h"

/* C types */
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
func_t     def_func(char *name, type_t type);
bool       is_type(val_t self);
bool       has_method(func_t self);
method_t   get_method(func_t func, int n);

/* convenience */
#define is_func(x)   has_type(x, &FuncType)
#define as_func(x)   ((func_t)as_obj(x))

#endif
