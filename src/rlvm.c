#include <stdlib.h>

#include "memory.h"
#include "opcodes.h"
#include "object.h"
#include "function.h"
#include "array.h"
#include "rlvm.h"



/* vm internal data structures */
control_t *make_control( void )
{
  return (control_t*)make_obj(&ControlType);
}

void init_control( control_t *control, closure_t *module, control_t *caller, size_t n )
{
  control->next        = caller;
  control->function    = module->function;
  control->environment = make_environment();

  init_environment(control->environment, false, false, module->environment, NULL, n);
}

void free_control( control_t *control )
{
  if ( !control->environment->is_captured )
    free_environment(control->environment);

  free_obj(&control->obj);
}

control_t *make_call_frame( control_t *caller, closure_t *module, size_t n )
{
  control_t *callee = make_control();

  init_control(callee, module, caller, n);

  envt_bind(caller->environment,
	    callee->environment->binds->data,
	    n);

  envt_unbind(caller->environment, n);

  return callee;
}

environment_t *make_environment( void )
{
  return (environment_t*)make_obj(&EnvironmentType);
}

void init_environment( environment_t *environment, bool is_lenvt, bool is_captured, environment_t *next, value_t *ini, size_t n )
{
  environment->is_lenvt    = is_lenvt;
  environment->is_captured = is_captured;
  environment->next        = next;
  environment->binds       = make_vector(n);

  init_vector(environment->binds, n, ini);
}

value_t envt_ref( environment_t *environment, size_t i, size_t j )
{
  while (i--)
    environment = environment->next;

  return vector_ref(environment->binds, j);
}

value_t envt_set( environment_t *environment, size_t i, size_t j, value_t value )
{
  while (i--)
    environment = environment->next;

  return vector_set(environment->binds, j, value);
}

void envt_bind( environment_t *environment, value_t *buffer, size_t n )
{
  vector_cpy(environment->binds, buffer, n);
}

void envt_unbind( environment_t *environment, size_t n )
{
  vector_pop(environment->binds, n);
}

void envt_capture( environment_t *environment )
{
  while ( environment && !environment->is_captured )
    {
      environment->is_captured = true;
      environment              = environment->next;
    }
}

/* main interpreter loop */
value_t rl_exec( closure_t *module )
{
  static void *labels[] =
    {
      [op_invalid]      =&&label_invalid,
      [op_nothing]      =&&label_nothing,
      
      [op_load_nul]     =&&label_load_nul,
      [op_load_true]    =&&label_load_true,
      [op_load_false]   =&&label_load_false,

      [op_load_value]   =&&label_load_value,
      [op_load_const]   =&&label_load_const,

      [op_load_global]  =&&label_load_global,
      [op_store_global] =&&label_store_global,
      [op_load_closure] =&&label_load_closure,
      [op_store_closure]=&&label_store_closure,

      [op_jump_true]    =&&label_jump_true,
      [op_jump_false]   =&&label_jump_false,
      [op_jump]         =&&label_jump,

      [op_invoke]       =&&label_invoke,
      [op_return]       =&&label_return,
      [op_closure]      =&&label_closure,

      [op_argco]        =&&label_argco,
      [op_vargco]       =&&label_vargco,

      [op_halt]         =&&label_halt
    };

  ushort op = 0;

  size_t argc;
  
  short  argx, argy;

  value_t x, y;

  control_t *control = make_call_frame(NULL, module, 0);
  environment_t *eframe;

 label_fetch:
  if ( rl_recover() )
    goto label_halt;

  op   = *(control->ip++);

  argc = op_argc(op);

  if (argc > 0)
    argx = *(control->ip++);

  if (argc > 1)
    argy = *(control->ip++);

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "error: invalid opcode %d.\n", op);
  fprintf(stderr, "aborting.\n");
  abort();

 label_nothing:
  goto label_fetch;

 label_load_nul:
  rl_push( NUL );

  goto label_fetch;

 label_load_true:
  rl_push( TRUE );

  goto label_fetch;

 label_load_false:
  rl_push( FALSE );

  goto label_fetch;

 label_load_value:
  x = control->function->constants->data[argx];
  
  rl_push( x );
  
  goto label_fetch;

 label_load_const:
  x = control->function->constants->data[argx];
  x = as_atom(x)->constant;

  rl_push( x );

  goto label_fetch;

 label_load_local:
  x = control->locals[argx];
  
  rl_push( x );

  goto label_fetch;

 label_store_local:
  x                     = rl_peek(0);
  control->locals[argx] = x;

  goto label_fetch;

 label_load_global:
  y = control->function->constants->data[argx];
  x = as_atom(y)->bind;

  rl_push(x);

  goto label_fetch;

 label_store_global:
  y                = control->function->constants->data[argx];
  as_atom(y)->bind = rl_peek(0);

  goto label_fetch;

 label_load_closure:
  eframe = control->environment;

  while (argx--)
    eframe = eframe->next;

  x = eframe->binds->data[argy];

  rl_push(x);

  goto label_fetch;

 label_store_closure:
  eframe = control->environment;

  while (argx--)
    eframe = eframe->next;

  eframe->binds->data[argy] = rl_peek(0);

  goto label_fetch;

 label_jump_true:
  x            = rl_pop();
  control->ip += argx*rl_to_C_bool(x);

  goto label_fetch;

 label_jump_false:
  x            = rl_pop();
  control->ip += argx*(!rl_to_C_bool(x));

  goto label_fetch;

 label_jump:
  control->ip += argx;

  goto label_fetch;

 label_invoke:
  x = rl_peek(argx);

  if ( is_pfunc(x) )
    {
      x = as_pfunc(x)(Vm.sp-argx, argx);

      rl_popn(argx);
      rl_push(x);
    }

  else if ( is_closure(x) )
      control = make_control( as_closure(x), control );

  else
    rl_panic( "expected a function, got <%s>", rl_typeof(x)->name );

  goto label_fetch;

 label_return:
  x = rl_popn( control->function->nstack+1 );

  free_control( &control );

  goto label_fetch;

 label_closure:
  x = rl_pop();  // lambda object
  

 label_argco:

 label_vargco:

 label_halt:
  
}
