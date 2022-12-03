#include <assert.h>

#include "def/opcodes.h"

#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/type.h"

#include "vm/memory.h"
#include "vm/obj/support/stack.h"
#include "vm/obj/support/envt.h"

/* commentary */

/* C types */

/* globals */
void init_control( object_t *object );
void trace_control( object_t *object );
void free_control( object_t *object );

datatype_t ControlType =
  {
    .type   = { gl_datatype_head, "control", datatype_isa, NULL },

    .vmtype = vmtype_objptr,
    .obsize = sizeof(control_t),
    .init   = init_control,
    .trace  = trace_control,
    .free   = free_control
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
control_t *make_control( lambda_t *func, envt_t *envt, value_t *args, int nargs, control_t *caller )
{
  control_t *control = (control_t*)make_object(&ControlType);

  control->function = func;
  control->ip       = func->instructions;
  control->envt     = envt;
  control->stack    = make_stack(nargs, args);
  control->caller   = caller;

  return control;
}

/* interfaces to internals */
value_t get_control_const( control_t *control, uint i )
{
  return get_lambda_const(control->function, i);
}

value_t get_control_envt_ref( control_t *control, size_t i, size_t j )
{
  return get_envt_ref(control->envt, i, j);
}

value_t set_control_envt_ref( control_t *control, size_t i, size_t j, value_t bind )
{
  return set_envt_ref(control->envt, i, j, bind);
}

/* stack interface */
size_t push_to_control_stack( control_t *control, value_t x )
{
  return stack_push(control->stack, x);
}

value_t pop_from_control_stack( control_t *control )
{
  return stack_pop(control->stack);
}

value_t peek_from_control_stack( control_t *control, long i )
{
  return stack_ref(control->stack, i);
}

value_t *control_stack_at( control_t *control, long i )
{
  return stack_at(control->stack, i);
}

/* ip interface */
ushort control_fetch( control_t *control, int *argx, int *argy )
{
  opcode_t out = *control->ip++;
  size_t  argc = op_argc(out);

  if ( argc > 0 )
    *argx = *control->ip++;

  if ( argc > 1 )
    *argy = *control->ip++;

  return out;  
}

ushort *control_jump( control_t *control, int argx )
{
  return (control->ip += argx);
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
