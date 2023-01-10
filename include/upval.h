#ifndef rl_upval_h
#define rl_upval_h

#include "obj.h"

/* C types */
struct upval_t {
  upval_t next;
  val_t   value;
  int     index;
  bool    closed;
};

/* globals */
extern type_t UpvalType;

/* API */
upval_t open_upval(int i, upval_t *o_u);
void    close_upval(int i, upval_t *o_u);
val_t   ref_upval(int i, vec_t u, vals_t *l);
val_t   set_upval(int i, val_t v, vec_t u, vals_t *l);

/* convenience */
#define is_upval(x)     has_type(x, &UpvalType)
#define as_upval(x)     ((upval_t)as_obj(x))
#define upval_next(x)   (as_upval(x)->next)
#define upval_value(x)  (as_upval(x)->value)
#define upval_index(x)  (as_upval(x)->index)
#define upval_closed(x) (as_upval(x)->closed)

#endif
