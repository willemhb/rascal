#include <stdlib.h>
#include <assert.h>

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

/* utilities/convenience */
size_t rl_push( value_t x )
{
  vector_t *locals = Vm.control->environment->binds;

  array_add(locals, 1, x);
  return locals->count-1;
}

value_t rl_pop( void )
{
  vector_t *locals = Vm.control->environment->binds;

  assert(locals->count > 0);
  
  value_t   out    = array_pop(locals, 1);

  return out;
}

size_t rl_pushn( size_t n )
{
  vector_t *locals = Vm.control->environment->binds;
  size_t    out    = locals->count;

  resize_array(locals, locals->count+n);

  return out;
}

value_t rl_popn( size_t n )
{
  vector_t *locals = Vm.control->environment->binds;
  value_t   out    = locals->data[locals->count-1];

  assert(n <= locals->count);
  resize_array(locals, locals->count-n);

  return out;
}

value_t rl_peek( long i )
{
  vector_t *locals = Vm.control->environment->binds;

  return array_ref(locals, i);
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

  environment_t *cenvt; lambda_t *lmb;

  Vm.control = make_call_frame(NULL, module, 0);

 label_fetch:
  if ( rl_recover() )
    goto label_halt;

  op   = *(Vm.control->ip++);

  argc = op_argc(op);

  if (argc > 0)
    argx = *(Vm.control->ip++);

  if (argc > 1)
    argy = *(Vm.control->ip++);

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "error: invalid opcode %d.\n", op);
  fprintf(stderr, "aborting.\n");
  abort();

 label_nothing:
  goto label_fetch;

 label_load_nul:
  rl_push(NUL);

  goto label_fetch;

 label_load_true:
  rl_push(TRUE);

  goto label_fetch;

 label_load_false:
  rl_push(FALSE);

  goto label_fetch;

 label_load_value:
  x = lmb_get_value(Vm.control->function, argx);

  rl_push(x);
  
  goto label_fetch;

 label_load_const:
  x = lmb_get_constant(Vm.control->function, argx);

  rl_push(x);

  goto label_fetch;

 label_load_global:
  x = lmb_get_constant(Vm.control->function, argx);

  rl_push(x);

  goto label_fetch;

 label_store_global:
  lmb_set_constant(Vm.control->function, argx, rl_peek(-1));

  goto label_fetch;

 label_load_closure:
  x = envt_ref(Vm.control->environment, argx, argy);

  rl_push(x);

  goto label_fetch;

 label_store_closure:
  x = envt_set(Vm.control->environment, argx, argy, rl_peek(-1));

  goto label_fetch;

 label_jump_true:
  x               = rl_pop();
  Vm.control->ip += argx*rl_to_C_bool(x);

  goto label_fetch;

 label_jump_false:
  x               = rl_pop();
  Vm.control->ip += argx*(!rl_to_C_bool(x));

  goto label_fetch;

 label_jump:
  Vm.control->ip += argx;

  goto label_fetch;

 label_invoke:
  x = rl_peek(-argx);

  if ( is_pfunc(x) )
    {
      x = as_pfunc(x)(Vm.control->environment->binds->data, argx);

      rl_popn(argx);
      rl_push(x);
    }

  else if ( is_closure(x) )
    {
      Vm.control = make_call_frame(Vm.control, as_closure(x), argx);
      argy       = argx;
    }

  else
    rl_panic("expected a function, got <%s>", rl_typeof(x)->name);

  goto label_fetch;

 label_return:
  x = rl_peek(-1);

  free_call_frame(&Vm.control);
  rl_push(x);

  goto label_fetch;

 label_closure:
  x      = rl_pop();  // lambda object
  lmb    = as_lambda(x);
  cenvt  = Vm.control->environment;
  module = make_closure();

  init_closure(module, lmb, cenvt);

  x      = tag_obj(module);

  rl_push(x);

  goto label_fetch;

 label_argco:
  if ( argx != argy )
    rl_panic("expected %d arguments to #, got %d", argx, argy);

  goto label_fetch;

 label_vargco:
  if ( argx < argy )
    rl_panic("expected at least %d arguments to #, got %d", argx, argy);

  argc = argy - argx;
  x    = list_n(argc, envt_at(-argc), argc);

  rl_popn(argc);
  rl_push(x);

  goto label_fetch;

 label_halt:
  x = rl_peek(-1);

  free_call_frame(&Vm.control);

  return x;
}
