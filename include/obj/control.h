#ifndef rl_obj_control_h
#define rl_obj_control_h

#include "vm/object.h"

#include "vm/obj/support/stack.h"

/* commentary

   Basically a first class callstack frame.

   For the time being, this seems like the most straightforward way to implement effects. */

/* C types */
struct rl_control_t
{
  rl_object_t obj;

  stack_t *stack;
  ushort *ip;

  rl_control_t *caller;
  rl_environment_t *environment;
  rl_bytecode_t *bytecode;
};

/* globals */
extern rl_datatype_t ControlType;

/* API */
/* constructors */

/* accessors */
/* function interface (interact with constant store) */

/* stack interface */

/* runtime */
void rl_obj_control_init( void );
void rl_obj_control_mark( void );
void rl_obj_control_cleanup( void );

/* convenience */
#define is_control( x )   (rl_typeof(x)==&ControlType)
#define as_control( x )   ((control_t*)((x)&PTRMASK))

#endif
