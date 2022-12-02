#ifndef rl_obj_control_h
#define rl_obj_control_h

#include "vm/object.h"

/* commentary

   Basically a first class callstack frame.

   For the time being, this seems like the most straightforward way to implement effects. */

/* C types */
struct control_t
{
  object_t   obj;

  control_t *caller;
  stack_t   *stack;
  envt_t    *envt;
  lambda_t  *function;
  ushort    *ip;
};

/* globals */
extern datatype_t ControlType;

/* API */
/* constructors */
control_t *make_control( lambda_t *function, envt_t *envt, value_t *args, size_t n_args, control_t *caller );

/* accessors */
control_t *get_control_caller( control_t *control );
symbol_t  *get_control_fname( control_t *control );

/* function interface (interact with constant store) */
value_t    get_control_const( control_t *control, uint i );
value_t    get_control_envt_ref( control_t *control, size_t i, size_t j );
value_t    set_control_envt_ref( control_t *control, size_t i, size_t j, value_t val );

/* stack interface */
size_t     push_to_control_stack( control_t *control, value_t x );
value_t    pop_from_control_stack( control_t *control );
value_t    popn_from_control_stack( control_t *control, size_t n );
value_t    peek_from_control_stack( control_t *control, long i );
value_t   *control_stack_at( control_t *control, long i );

/* ip interface */
ushort  control_fetch( control_t *control, int *argx, int *argy );
ushort *control_jump( control_t *control, int argx );

/* runtime */
void rl_obj_control_init( void );
void rl_obj_control_mark( void );
void rl_obj_control_cleanup( void );

/* convenience */
#define is_control( x )   (rl_typeof(x)==&ControlType)
#define as_control( x )   ((control_t*)((x)&PTRMASK))

#endif
