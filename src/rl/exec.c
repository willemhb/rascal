#include <stdlib.h>

#include "def/opcodes.h"

#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/native.h"
#include "obj/type.h"

#include "vm/error.h"
#include "vm/value.h"

#include "vm/obj/vm.h"
#include "vm/obj/support/stack.h"

#include "rl/exec.h"

/* commentary */

/* C types */

/* globals */

/* API */

value_t exec( lambda_t *code )
{
  static void *labels[] =
    {
     [op_invalid]     = &&label_invalid,
     [op_halt]        = &&label_halt,
     [op_nothing]     = &&label_noop,
     [op_pop]         = &&label_pop,
     [op_load_const]  = &&label_load_const,
     [op_load_global] = &&label_load_global,
     [op_store_global]= &&label_store_global,
     [op_invoke]      = &&label_invoke
    };

  opcode_t op;

  size_t argc;
  
  ushort argx;

  value_t x, y, v, *a;

  Vm.executing = make_control(code);

 label_dispatch:
  if ( recover() )
    goto label_abort;
  
  op   = *Vm.executing->ip++;
  argc = op_argc(op);

  if (argc > 0)
    argx = *Vm.executing->ip++;

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "Invalid opcode.\n");
  abort();

 label_abort:
  free_object((object_t*)Vm.executing);
  Vm.executing = NULL;
  return NUL;

 label_halt:
  v = stack_pop(Vm.executing->stack);
  free_object((object_t*)Vm.executing);
  Vm.executing = NULL;
  return v;

 label_noop:
  goto label_dispatch;

 label_pop:
  stack_pop(Vm.executing->stack);

  goto label_dispatch;

 label_load_const:
  x = get_const(Vm.executing->function, argx);
  stack_push(Vm.executing->stack, x);

  goto label_dispatch;

 label_load_global:
  y = get_const(Vm.executing->function, argx);
  x = as_symbol(y)->bind;
  stack_push(Vm.executing->stack, x);

  goto label_dispatch;

 label_store_global:
  y = get_const(Vm.executing->function, argx);
  x = stack_ref(Vm.executing->stack, -1);
  as_symbol(y)->bind = x;

  goto label_dispatch;

 label_invoke:
  x = stack_ref(Vm.executing->stack, -argx-1);

  if ( is_native(x) )
    goto label_invoke_native;

  panic("Don't know how to apply value of type %s.\n", type_name(rl_typeof(x)));
  goto label_dispatch;

 label_invoke_native:
  a = (value_t*)Vm.executing->stack+Vm.executing->stack->len-argx-1;
  y = as_native(x)(a, argx);

  stack_popn(Vm.executing->stack, argx);

  if ( recover() )
    goto label_abort;
  
  stack_push(Vm.executing->stack, y);
  goto label_dispatch;
}

/* runtime */
void rl_rl_exec_init( void ) {}
void rl_rl_exec_mark( void ) {}

/* convenience */
