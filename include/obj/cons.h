#ifndef rl_obj_cons_h
#define rl_obj_cons_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
struct cons_t
{
  object_t obj;

  value_t  car;
  value_t  cdr;
};

/* globals */
extern datatype_t ConsType;

/* API */
cons_t *cons(value_t car, value_t cdr);

/* runtime */
void rl_obj_nul_init( void );
void rl_obj_nul_mark( void );

/* convenience */
#define is_nul( x ) ((x)==NUL)
#define as_nul( x ) (NULL)

#endif
