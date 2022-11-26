#ifndef rascal_obj_control_h
#define rascal_obj_control_h

#include "obj/object.h"

/* C types */
struct control_data_t
{
  vector_t  stack;
  lambda_t  function;
  ushort   *ip;
};

struct control_t
{
  struct object_t obj;
  struct control_data_t data;
};

/* globals */
extern struct type_t ControlType;

/* runtime */
void rl_obj_control_init( void );

/* API */
control_t make_control( lambda_t function );
void      free_control( control_t control );
value_t   control_constant( control_t control, size_t n );
size_t    control_push( control_t control, value_t value );
value_t   control_pop( control_t control );
value_t   control_peek( control_t control, long i );

/* convenience */
#define is_control( x )   (rl_typeof(x)==&ControlType.data)
#define as_control( x )   ((control_t)((x)&PTRMASK))

#endif
