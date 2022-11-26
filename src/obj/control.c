#include "obj/control.h"
#include "obj/type.h"
#include "obj/lambda.h"
#include "obj/vector.h"

/* commentary */

/* C types */

/* globals */
struct type_t ControlType =
  {
    { .type=&TypeType.data, .size=sizeof(struct type_t) },

    {
      .name="control",
      .vmtype=vmtype_object,
      .obsize=sizeof(struct control_t),
      .elsize=0,
      .stringp=false
    }
  };

/* API */
control_t make_control( lambda_t function )
{
  control_t control = (control_t)make_object(&ControlType.data);

  control->stack    = make_vector(0, NULL);
  control->function = function;
  control->ip       = function->code;

  return control;
}

void free_control( control_t control )
{
  free_vector(control->stack);
  free_object((object_t)control);
}

value_t control_constant( control_t control, size_t n )
{
  return lambda_constant(control->function, n);
}

size_t control_push( control_t control, value_t x )
{
  return vector_add(control->stack, 1, x);
}

value_t control_pop( control_t control )
{
  return vector_pop(control->stack, 1);
}

value_t control_peek( control_t control, long i )
{
  return vector_ref(control->stack, i);
}

/* runtime */
void rl_obj_vector_init( void ) {}

/* convenience */
