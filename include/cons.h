#ifndef rascal_pair_h
#define rascal_pair_h

#include "object.h"
#include "value.h"

// cons flags
#define PROPER_FL  0x0000000000000004ul
#define INLINED_FL 0x0000000000000008ul

struct cons_t
{
  object_t object;

  type_t   type;
  arity_t  length;

  value_t  space[0];
};

#define is_cons(val)     isa(val, CONS)
#define as_cons(val)     ((cons_t*)as_ptr(val))
#define cons_car(val)    (as_cons(val)->car)
#define cons_cdr(val)    (as_cons(val)->cdr)

// forward declarations
cons_t   *new_cons(void);
cons_t   *new_conses(arity_t n);
void      init_conses(cons_t *cons, value_t *args, arity_t n);
void      init_cons(cons_t *cons, value_t car, value_t cdr);
void      mark_cons(object_t *obj);

void      prin_list(stream_t *port, value_t value);


#endif
