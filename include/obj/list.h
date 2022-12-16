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

struct rl_cons_t
{
  rl_object_t obj;

  rl_value_t  car;
  rl_value_t  cdr;
};

/* globals */
extern rl_datatype_t  ConsType, NulType;
extern rl_uniontype_t ListType;

/* API */
/* constructors */
rl_value_t make_cons(rl_value_t car, rl_value_t cdr);
rl_value_t make_conses(size_t n, ...);
rl_value_t make_list(size_t n, ...);

rl_value_t vec_to_cons( rl_value_t *args, size_t n );
rl_value_t vec_to_list( rl_value_t *args, size_t n );

/* accessors */
rl_value_t get_cons_car( rl_cons_t *cons );
rl_value_t set_cons_car( rl_cons_t *cons, rl_value_t x );
rl_value_t get_cons_cdr( rl_cons_t *cons );
rl_value_t set_cons_cdr( rl_cons_t *cons, rl_value_t x );

/* list accessors */
rl_value_t get_cons_head( rl_cons_t *cons );
rl_cons_t *get_rl_cons_tail( rl_cons_t *cons );

/* runtime dispatch */
void rl_obj_cons_init( void );
void rl_obj_cons_mark( void );
void rl_obj_cons_cleanup( void );

/* convenience */
#define is_cons( x ) (rl_typeof(x)==&ConsType)
#define as_cons( x ) ((rl_cons_t*)as_object(x))
#define is_nul( x )  ((x)==NUL)

#define get_cons_cadr( x )   get_cons_car(get_rl_cons_tail(x))
#define get_cons_cddr( x )   get_cons_cdr(get_rl_cons_tail(x))
#define get_cons_caddr( x )  get_cons_car(get_rl_cons_tail(get_rl_cons_tail(x)))
#define get_cons_cdddr( x )  get_cons_cdr(get_rl_cons_tail(get_rl_cons_tail(x)))
#define get_cons_cadddr( x ) get_cons_car(get_rl_cons_tail(get_rl_cons_tail(get_rl_cons_tail(x))))

size_t  cons_len( rl_cons_t *cons );
bool    is_proper( rl_value_t x );
bool    is_arglist( rl_value_t x );

#endif
