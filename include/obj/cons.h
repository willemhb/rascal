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
value_t cons(value_t car, value_t cdr);
value_t cons_n(size_t n, ...);

#define list_n( n, ... ) cons_n((n) __VA_ARGS__ __VA_OPT__(,) NUL )

value_t cons_v( value_t *args, size_t n);

/* runtime */
void rl_obj_cons_init( void );
void rl_obj_cons_mark( void );

/* convenience */
#define is_cons( x ) (rl_typeof(x)==&ConsType)
#define as_cons( x ) ((cons_t*)as_object(x))

#endif
