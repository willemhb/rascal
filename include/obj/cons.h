#ifndef rl_obj_cons_h
#define rl_obj_cons_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef enum cons_fl_t
  {
    cons_fl_proper  =0x0100,
    cons_fl_arglist =0x0200,
  } cons_fl_t;

struct cons_t
{
  object_t obj;

  value_t  car;
  value_t  cdr;
};

/* globals */
extern datatype_t ConsType;

/* API */
/* constructors */
value_t make_cons(value_t car, value_t cdr);
value_t make_conses(size_t n, ...);
value_t make_list(size_t n, ...);

value_t vec_to_cons( value_t *args, size_t n );
value_t vec_to_list( value_t *args, size_t n );

/* accessors */
value_t get_cons_car( cons_t *cons );
value_t set_cons_car( cons_t *cons, value_t x );
value_t get_cons_cdr( cons_t *cons );
value_t set_cons_cdr( cons_t *cons, value_t x );

/* list accessors */
value_t get_cons_head( cons_t *cons );
cons_t *get_cons_tail( cons_t *cons );

/* runtime dispatch */
void rl_obj_cons_init( void );
void rl_obj_cons_mark( void );
void rl_obj_cons_cleanup( void );

/* convenience */
#define is_cons( x ) (rl_typeof(x)==&ConsType)
#define as_cons( x ) ((cons_t*)as_object(x))

#define get_cons_cadr( x )   get_cons_car(get_cons_tail(x))
#define get_cons_cddr( x )   get_cons_cdr(get_cons_tail(x))
#define get_cons_caddr( x )  get_cons_car(get_cons_tail(get_cons_tail(x)))
#define get_cons_cdddr( x )  get_cons_cdr(get_cons_tail(get_cons_tail(x)))
#define get_cons_cadddr( x ) get_cons_car(get_cons_tail(get_cons_tail(get_cons_tail(x))))

size_t  cons_len( cons_t *cons );
bool    is_proper( value_t x );
bool    is_arglist( value_t x );

#endif
