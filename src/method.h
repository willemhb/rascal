#ifndef rl_method_h
#define rl_method_h

#include "obj.h"
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
method_t   make_method(int n, bool v, guard_fn_t g, val_t h);
val_t      method(int n, bool v, guard_fn_t g, val_t h);
eval_err_t validate_method_call(method_t m, int n, val_t *a);
bool       is_variadic(method_t m);

/* convenience */
#define is_method(x) has_type(x, &MethodType)
#define as_method(x) ((method_t)as_obj(x))

#define def_method(s, n, v, g, h)                                       \
  do {                                                                  \
    /* nb: def_func is idempotent. define types before adding methods. */ \
    func_t __f__ = def_func(s, NULL);                                   \
    add_method(__f__, n, v, g, value(h));                               \
  } while (0)

#endif
