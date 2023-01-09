#ifndef rl_method_h
#define rl_method_h

#include "obj.h"
#include "sym.h"
#include "error.h"

/* C types */
struct method_t {
  int        nargs;
  bool       vargs;
  guard_fn_t guard_fn;
  val_t      handler;
};

/* globals */
extern struct type_t MethodType;

/* API */
eval_err_t bind_method(func_t f, int n, bool v, guard_fn_t g, val_t h);
eval_err_t validate_method_call(method_t m, int n, val_t *a);

/* convenience */
#define native_method(name, nargs, vargs, guard, Cfun)                  \
  do {                                                                  \
    val_t __h__      = tag_val(Cfun, NATIVE);                           \
    val_t __fv__     = lookup(name);                                    \
    func_t __ff__    = as_func(__fv__);                                 \
    eval_err_t __s__ = bind_method(__ff__, nargs, vargs, guard, __h__); \
    if (__s__)                                                          \
      panic(__ff__, "error binding method");                            \
  } while (false)

#define prim_method(name, nargs, vargs, guard, label)                   \
  do {                                                                  \
    val_t __h__      = tag_val(label, SMALL);                           \
    val_t __fv__     = lookup(name);                                    \
    func_t __ff__    = as_func(__fv__);                                 \
    eval_err_t __s__ = bind_method(__ff__, nargs, vargs, guard, __h__); \
    if (__s__)                                                          \
      panic(__ff__, "error binding method");                            \
  } while (false)

#endif
