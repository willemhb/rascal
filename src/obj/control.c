#include <assert.h>

#include "def/opcodes.h"

#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/type.h"

#include "vm/obj/support/stack.h"

/* commentary */

/* C types */

/* globals */
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
/* object runtime */
void init_control( object_t *object )
{
  control_t *control = (control_t*)object;

  control->stack = make_stack(0, NULL);
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

/* constructors */
control_t *make_control( lambda_t *function )
{
  control_t *control = (control_t*)make_object(&ControlType);

  control->function = function;
  control->ip       = function->instructions;

  return control;
}

/* interfaces to internals */
value_t get_control_const( control_t *control, uint i )
{
  return get_lambda_const(control->function, i);
}

value_t get_control_global( control_t *control, uint i )
{
  value_t name = get_lambda_const(control->function, i);
  
  assert(is_symbol(name));

  return get_symbol_bind(as_symbol(name));
}

value_t set_control_global( control_t *control, uint i, value_t x )
{
  value_t name = get_lambda_const(control->function, i);

  assert(is_symbol(name));

  return set_symbol_bind(as_symbol(name), x);
}

/* stack interface */
size_t control_push( control_t *control, value_t x )
{
  return stack_push(control->stack, x);
}

value_t control_pop( control_t *control )
{
  return stack_pop(control->stack);
}

value_t control_peek( control_t *control, long i )
{
  return stack_ref(control->stack, i);
}

value_t *control_at( control_t *control, long i )
{
  return stack_at(control->stack, i);
}

/* ip interface */
ushort control_fetch( control_t *control, int *argx )
{
  opcode_t out = *control->ip++;
  size_t  argc = op_argc(out);

  if (argc > 0)
    *argx = *control->ip++;

  return out;  
}

/* runtime */
void rl_obj_control_init( void )
{
  gl_init_type(ControlType);
}

void rl_obj_control_mark( void )
{
  gl_mark_type(ControlType);
}

void rl_obj_control_cleanup( void ) {}

/* convenience */
