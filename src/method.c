#include "method.h"
#include "type.h"
#include "small.h"
#include "module.h"
#include "native.h"

/* C types */
struct method_init_t {
  int n;
  bool v;
  guard_fn_t g;
  val_t h;
};

/* globals */
void prin_method(val_t x);
void init_method(obj_t s, type_t t, int n, void *i);

struct type_t MethodType = {
  .name="method",
  .idno=method_type_idno,
  .prin=prin_method,
  .init=init_method,

  .head_size=sizeof(struct obj_head_t),
  .body_size=sizeof(struct method_t)
};

/* API */
/* external */
method_t make_method(int n, bool v, guard_fn_t g, val_t h) {
  struct method_init_t ini = { n, v, g, h };

  return (method_t)make_obj(&MethodType, 4, &ini);
}

val_t method(int n, bool v, guard_fn_t g, val_t h) {
  method_t out = make_method(n, v, g, h);

  return tag_val(out, OBJECT);
}

eval_err_t validate_method_call(method_t m, int n, val_t *a) {
  if (m == NULL)
    return no_method_err;

  if (n < m->nargs)
    return arity_underflow_err;

  if (n > m->nargs && !is_variadic(m))
    return arity_overflow_err;

  return m->guard_fn(n, a);
}

bool is_variadic(method_t m) {
  return m && m->vargs;
}

/* internal */
void prin_method(val_t x) {
  method_t m = as_method(x);

  if (is_small(m->handler))
    printf("<primitive method>");

  else if (is_native(m->handler))
    printf("<native method>");

  else
    printf("<user method>");
}

void init_method(obj_t s, type_t t, int n, void *i) {
  (void)t;
  (void)n;
  
  method_t m = (method_t)s;

  struct method_init_t *m_i = i;

  m->nargs    = m_i->n;
  m->vargs    = m_i->v;
  m->guard_fn = m_i->g;
  m->handler  = m_i->h;
}
