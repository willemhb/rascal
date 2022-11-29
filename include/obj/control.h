#ifndef rl_obj_control_h
#define rl_obj_control_h

#include "vm/object.h"

/* C types */
struct control_t
{
  object_t  obj;

  stack_t  *stack;
  lambda_t *function;
  ushort   *ip;
};

/* globals */
extern datatype_t ControlType;

/* runtime */
void rl_obj_control_init( void );
void rl_obj_control_mark( void );

/* API */
control_t *make_control( lambda_t *function );
value_t    cntl_constant( control_t *control, size_t n );
size_t     cntl_push( control_t *control, value_t value );
value_t    cntl_pop( control_t *control );
value_t    cntl_peek( control_t *control, long i );
void       cntl_fetch( control_t *control, ushort *op, ushort *x, ushort *y );

/* convenience */
#define is_control( x )   (rl_typeof(x)==&ControlType)
#define as_control( x )   ((control_t*)((x)&PTRMASK))

#endif
