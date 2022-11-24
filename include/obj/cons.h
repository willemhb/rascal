#ifndef rascal_cons_h
#define rascal_cons_h

#include "object.h"

/* C type */
struct cons_t
{
  OBJHEAD;

  value_t car;
  value_t cdr;
};

/* globals */
extern type_t ConsType, NulType;

/* API */
value_t cons( value_t car, value_t cdr );
value_t assoc( value_t key, value_t list );
value_t cons_n( size_t n, ... );
value_t list_n( size_t n, ... );

/* initialization */
void rl_cons_init( void );

/* utilities & convenience */
#define as_cons( x ) ((cons_t*)as_object(x))
#define is_cons( x ) value_is_type(x, &ConsType)

#endif
