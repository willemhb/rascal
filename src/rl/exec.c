#include <stdlib.h>

#include "def/opcodes.h"

#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/native.h"
#include "obj/type.h"
#include "obj/closure.h"
#include "obj/bool.h"

#include "vm/error.h"
#include "vm/value.h"

#include "vm/obj/vm.h"
#include "vm/obj/support/stack.h"

#include "rl/exec.h"

/* commentary */

/* C types */

/* globals */

/* API */
void push_call_frame( vm_t *vm, closure_t *closure, int nargs );

value_t exec( lambda_t *code )
{
  static void *labels[] =
    {
     [op_invalid]       = &&label_invalid,
     [op_halt]          = &&label_halt,
     [op_nothing]       = &&label_noop,
     [op_pop]           = &&label_pop,
     [op_load_const]    = &&label_load_const,
     [op_load_variable] = &&label_load_variable,
     [op_store_variable]= &&label_store_variable,
     [op_jump_true]     = &&label_jump_true,
     [op_jump_false]    = &&label_jump_false,
     [op_jump]          = &&label_jump,
     [op_invoke]        = &&label_invoke,
     [op_return]        = &&label_return,
     [op_make_closure]  = &&label_make_closure,
    };

  opcode_t op;

  size_t argc;
  
  int argx, argy;

  value_t x, y, v, *a;

  Vm.executing = make_control(code, Vm.toplevel_binds, NULL, 0, NULL);

 label_dispatch:
  if ( recover() )
    goto label_abort;

  op = control_fetch(Vm.executing, &argx, &argy);

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "Invalid opcode.\n");
  abort();

 label_abort:
  Vm.executing = Vm.executing->caller;
  return NUL;

 label_halt:
  v = pop_from_control_stack(Vm.executing);
  Vm.executing = Vm.executing->caller;
  return v;

 label_noop:
  goto label_dispatch;

 label_pop:
  pop_from_control_stack(Vm.executing);

  goto label_dispatch;

 label_load_const:
  x = get_control_const(Vm.executing, argx);
  push_to_control_stack(Vm.executing, x);

  goto label_dispatch;

 label_load_variable:
  x = get_control_envt_ref(Vm.executing, argx, argy);
  push_to_control_stack(Vm.executing, x);

  goto label_dispatch;

 label_store_variable:
  x = peek_from_control_stack(Vm.executing, -1);
  set_control_envt_ref(Vm.executing, argx, argy, x);

  goto label_dispatch;

 label_jump_true:
  x = pop_from_control_stack(Vm.executing);

  if ( as_cbool(x) )
    control_jump(Vm.executing, argx);

  goto label_dispatch;

 label_jump_false:
  x = pop_from_control_stack(Vm.executing);

  if ( !as_cbool(x) )
    control_jump(Vm.executing, argx);

  goto label_dispatch;

 label_jump:
  control_jump(Vm.executing, argx);

  goto label_dispatch;

 label_make_closure:
  y = pop_from_control_stack(Vm.executing);
  x = capture_closure(as_lambda(y), Vm.executing->envt);

  push_to_control_stack(Vm.executing, x);

  goto label_dispatch;

 label_invoke:
  x = peek_from_control_stack(Vm.executing, -(argx+1));

  if ( is_native(x) )
    goto label_invoke_native;

  if ( is_closure(x) )
    goto label_invoke_closure;

  if ( is_control(x) )
    {
      v = pop_from_control_stack(Vm.executing);
      goto label_invoke_control;
    }

  panic("Don't know how to apply value of type %s", get_datatype_name(rl_typeof(x)));
  goto label_dispatch;

 label_invoke_native:
  a = control_stack_at(Vm.executing, -(argx+1));
  y = as_native(x)(a, argx);
  
  popn_from_control_stack(Vm.executing, argx);

  if ( recover() )
    goto label_abort;
  
  stack_push(Vm.executing->stack, y);
  goto label_dispatch;

 label_invoke_closure:
  argc = argx;

  check_closure_argco(as_closure(x), argc);

  if ( !panicking() )
    push_call_frame(&Vm, as_closure(x), argc);

  goto label_dispatch;

 label_return:
  v            = pop_from_control_stack(Vm.executing);
  Vm.executing = Vm.executing->caller;
  
  push_to_control_stack(Vm.executing, v);

  goto label_dispatch;

 label_invoke_control:
  v            = pop_from_control_stack(Vm.executing);
  Vm.executing = as_control(x);

  push_to_control_stack(Vm.executing, v);

  goto label_dispatch;
}

/* runtime */
void rl_rl_exec_init( void )    {}
void rl_rl_exec_mark( void )    {}
void rl_rl_exec_cleanup( void ) {}

/* convenience */
