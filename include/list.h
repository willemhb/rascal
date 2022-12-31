#ifndef rl_list_h
#define rl_list_h

#include "obj.h"

/* C types */

struct cons_t {
  val_t car, cdr;
};

/* globals */
extern struct type_t ConsType, ListType;

#define NUL OBJECT

/* API */
cons_t make_cons(val_t car, val_t cdr);
cons_t make_conses(size_t n, val_t *args);
cons_t make_list(size_t n, val_t *args);
val_t  cons(val_t car, val_t cdr);
val_t  list(size_t n, ...);
size_t list_len(val_t xs);

/* convenience */
#define is_cons(x) has_type(x, &ConsType)
#define as_cons(x) ((cons_t)as_obj(x))

#define is_list(x) has_type(x, &ListType)

#define car(x)    (as_cons(x)->car)
#define cdr(x)    (as_cons(x)->cdr)
#define cddr(x)   cdr(cdr(x))
#define cdar(x)   cdr(car(x))
#define cadr(x)   car(cdr(x))
#define caar(x)   car(car(x))

#define caddr(x)  car(cdr(cdr(x)))

#define cadddr(x) car(cdr(cdr(cdr(x))))

#endif
