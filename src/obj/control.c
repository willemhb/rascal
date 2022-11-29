#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/type.h"


#include "vm/obj/support/stack.h"

/* commentary */

/* C types */

/* globals */
#define N_STACK 32

void init_control( object_t *object );
void trace_control( object_t *object );
void free_control( object_t *object );

struct vtable_t ControlMethods =
  {
    .init=init_control,
    .trace=trace_control,
    .free=free_control
  };

struct layout_t ControlLayout =
  {
    .vmtype=vmtype_objptr,
    .flags=0,
    .obsize=sizeof(control_t)
  };

struct datatype_t ControlType =
  {
    {
      .obj=obj_init( &TypeType, sizeof(struct datatype_t), object_fl_static ),
      .name="control"
    },

    .methods=&ControlMethods,
    .layout=&ControlLayout
  };

/* API */
control_t *make_control( lambda_t *function )
{
  control_t *control = (control_t*)make_object(&ControlType);

  control->function = function;
  control->ip       = function->instructions;

  return control;
}

void init_control( object_t *object )
{
  control_t *control = (control_t*)object;

  control->stack = make_stack(N_STACK, NULL);
  control->function = NULL;
  control->ip = NULL;
}

void trace_control( object_t *object )
{
  control_t *control =(control_t*)object;
  
  mark_stack(control->stack);
  mark_object((object_t*)control->function);
}

void free_control( object_t *object )
{
  control_t *control = (control_t*)object;

  free_stack(control->stack);
}

/* runtime */
void rl_obj_control_init( void )
{
  init_object(&ControlType.type.obj);
}

void rl_obj_control_mark( void )
{
  init_object(&ControlType.type.obj);
}

/* convenience */
