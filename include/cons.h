#ifndef rascal_cons_h
#define rascal_cons_h

#include "object.h"

struct cons_t
{
  object_t object;

  arity_t  len;
  bool     isProper;

  value_t  car;

  union
  {
    value_t cdr;
    cons_t *tail;
  };
};

// api
cons_t *new_cons( void );
void    init_cons( cons_t *c, value_t car, value_t cdr );
void    mark_cons( object_t *object );
hash_t  hash_cons( object_t *object );
ord_t   order_conses( object_t *x, object_t *y );

// convenience
#define as_cons(val)   ((cons_t*)as_ptr(val))

#define cons_car(val)      (as_cons(val)->car)
#define cons_cdr(val)      (as_cons(val)->cdr)
#define cons_tail(val)     (as_cons(val)->tail)
#define cons_len(val)      (as_cons(val)->len)
#define cons_isProper(val) (as_cons(val)->isProper)


static inline REPR_PRED(cons, CONS);
static inline VAL_PRED(nul, NUL_VAL);

static inline bool isProper( value_t x )
{
  return is_cons(x)
    && (is_nul(x) || cons_isProper(x));
}

#endif
