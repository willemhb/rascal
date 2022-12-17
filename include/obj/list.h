#ifndef rl_obj_cons_h
#define rl_obj_cons_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef enum cons_fl_t
  {
    cons_fl_proper  =1,
  } cons_fl_t;

struct rl_cons_t
{
  rl_object_t obj;

  union {
    struct {
      rl_value_t car;
      rl_value_t cdr;
    } dotted;

    struct {
      rl_value_t head;
      rl_cons_t *tail;
    } proper;
  };
};

/* globals */
/* API */
/* constructors */
rl_value_t make_cons( rl_value_t car, rl_value_t cdr );
rl_value_t make_conses( size_t n, ... );

rl_value_t vec_to_cons( rl_value_t *args, size_t n );

/* accessors */
rl_value_t list_head( rl_cons_t *cons );
rl_cons_t *list_tail( rl_cons_t *);

/* runtime dispatch */
void rl_obj_cons_init( void );
void rl_obj_cons_mark( void );
void rl_obj_cons_cleanup( void );

/* convenience */
#define is_cons( x ) has_object_type(x, cons_object)
#define as_cons( x ) ((rl_cons_t*)as_object(x))
#define is_nul( x )  ((x)==NUL)

#define car( x )  (as_cons(x)->dotted.car)
#define cdr( x )  (as_cons(x)->dotted.cdr)
#define head( x ) (as_cons(x)->proper.head)
#define tail( x ) (as_cons(x)->proper.tail)

size_t  cons_len( rl_cons_t *cons );
bool    is_proper( rl_value_t x );

#endif
