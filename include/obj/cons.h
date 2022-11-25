#ifndef rl_obj_cons_h
#define rl_obj_cons_h

#include "obj/object.h"

/* C type */
typedef enum cons_fl_t
  {
    cons_fl_proper = 1,
  } cons_fl_t;

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
value_t assoca( value_t key, value_t list );
value_t assocp( value_t key, value_t list );
value_t cons_n( size_t n, ... );

#define list_n( n, ... ) cons_n((n),__VA_ARGS__ __VA_OPT__(,) NUL)

/* runtime */
void rl_obj_cons_init( void );
void rl_obj_cons_mark( void );

/* convenience */
static inline bool    is_nul( value_t x )    { return x == NUL; }
static inline bool    is_cons( value_t x )   { return rl_isa(x, &ConsType); }
static inline bool    is_list( value_t x )   { return is_nul(x) || is_cons(x); }

static inline cons_t *as_cons( value_t x )   { return (cons_t*)as_object(x); }

static inline bool    is_proper( value_t x )
{
  if ( is_nul(x) )
    return true;

  return is_cons(x) && fl_pred(as_object(x)->flags, cons_fl_proper);
}

#define cons_car( x )  (as_cons(x)->car)
#define cons_cdr( x )  (as_cons(x)->cdr)
#define cons_cddr( x ) cons_cdr(cons_cdr(x))
#define cons_cdar( x ) cons_cdr(cons_car(x))
#define cons_cadr( x ) cons_car(cons_cdr(x))
#define cons_caar( x ) cons_car(cons_car(x))

#endif
