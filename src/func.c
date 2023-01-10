#include <stdlib.h>

#include "func.h"

#include "type.h"
#include "memory.h"

#include "prin.h"

/* C types */
struct func_init_t {
  char   *n;
  type_t  t;
};

/* globals */
void prin_func(val_t x);
void init_func(obj_t s, type_t t, int n, void *i);
void free_func(obj_t s);

struct type_t FuncType = {
  .name="func",
  .idno=func_type_idno,

  .prin=prin_func,
  .init=init_func,
  .free=free_func,

  .head_size=sizeof(struct obj_head_t),
  .body_size=sizeof(struct func_t)
};

/* API */
/* internal */
void prin_func(val_t x) {
  char *prefix = is_type(x) ? "type" : "func";

  printf("#%s<%s>", prefix, func_name(x));
}

void init_func(obj_t s, type_t t, int n, void *i) {
  (void)t;
  (void)n;

  struct func_init_t *f_i = i;

  func_name(s)     = f_i->n;
  func_type(s)     = f_i->t;
  func_vmethod(s)  = NULL;
  func_fmethods(s) = make_objs(0, NULL);
}

void free_func(obj_t s) {
  free_objs(func_fmethods(s));
}

/* external */
#include "method.h"

func_t make_func(char *name, type_t type) {
  struct func_init_t fi = { name, type };

  return (func_t)make_obj(&FuncType, 2, &fi);
}

val_t func(char *name, type_t type) {
  return value(make_func(name, type));
}

func_t def_func(char *name, type_t type) {
  val_t s = sym(name);

  if (!is_bound(s))
    bind(s, func(name, type));

  return as_func(sym_val(s));
}

method_t get_method(func_t f, int n) {
  if ((size_t)n > func_fmethods(f)->len)
    return func_vmethod(f);

  return (method_t)objs_ref(func_fmethods(f), n);
}

method_t add_method(func_t f, int n, bool v, guard_fn_t g, val_t h) {
  if (has_method(f, n))
    return NULL;

  method_t m = make_method(n, v, g, h);

  if (v)
    func_vmethod(f) = m;

  else {
    if ((size_t)n > func_fmethods(f)->len) {
      size_t o_l = func_fmethods(f)->len;
      size_t n_l = objs_trim(func_fmethods(f), n);

      for (size_t i=o_l; i<n_l; i++)
        objs_set(func_fmethods(f), i, NULL);
    }

    objs_set(func_fmethods(f), n, (obj_t)m);
  }

  return m;
}

eval_err_t validate_func_call(func_t f, int n, val_t *a) {
  method_t m = get_method(f, n);

  if (m == NULL)
    return no_method_err;

  return validate_method_call(m, n, a);
}
