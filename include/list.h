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

/* initialization */
void cons_init(void);

/* convenience */
#define is_cons(x) has_type(x, &ConsType)
#define as_cons(x) ((cons_t)as_obj(x))

#define is_list(x) has_type(x, &ListType)

#endif
