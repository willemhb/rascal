#ifndef rl_list_h
#define rl_list_h

#include "obj.h"

/* C types */

struct cons_t {
  val_t car, cdr;
};

/* globals */
#define NUL OBJECT

/* API */
cons_t make_cons(val_t car, val_t cdr);
cons_t make_conses(size_t n, val_t *args);
cons_t make_list(size_t n, val_t *args);
val_t  cons(val_t car, val_t cdr);

#include "tpl/decl/generic.h"

bool METHOD(val, is_cons)(val_t val);
bool METHOD(obj, is_cons)(obj_t obj);
bool METHOD(cons, is_cons)(cons_t cons);

cons_t METHOD(val, as_cons)(val_t val);
cons_t METHOD(obj, as_cons)(obj_t obj);
cons_t METHOD(cons, as_cons)(cons_t cons);

#define is_cons(x)  GENERIC_CALL_3(is_cons, val, obj, cons, x)
#define as_cons(x)  GENERIC_CALL_3(as_cons, val, obj, cons, x)
#define cons_car(x) GETF_3(car, as_cons, val, obj, cons, x)
#define cons_cdr(x) GETF_3(cdr, as_cons, val, obj, cons, x)

/* initialization */
void cons_init(void);

/* misc */

#endif
