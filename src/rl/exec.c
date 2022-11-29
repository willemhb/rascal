#include <stdlib.h>

#include "def/opcodes.h"

#include "obj/control.h"
#include "obj/lambda.h"

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
     [op_invalid]   = &&label_invalid,
     [op_halt]      = &&label_halt,
     [op_nothing]   = &&label_noop,
     [op_load_const]= &&label_load_const,
    };

  opcode_t op;

  size_t argc;
  
  ushort argx, argy;

  value_t x, v;

  Vm.executing = make_control(code);

 label_dispatch:
  op   = *Vm.executing->ip++;
  argc = op_argc(op);

  if (argc > 0)
    argx = *Vm.executing->ip++;

  if (argc > 1)
    argy = *Vm.executing->ip++;

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "Invalid opcode.\n");
  abort();

 label_halt:
  v = stack_pop(Vm.executing->stack);
  free_object((object_t*)Vm.executing);
  Vm.executing = NULL;
  return v;

 label_noop:
  goto label_dispatch;

 label_load_const:
  x = get_const(Vm.executing->function, argx);
  stack_push(Vm.executing->stack, x);

  goto label_dispatch;
}

/* runtime */
void rl_vm_exec_init( void ) {}
void rl_vm_exec_mark( void ) {}

/* convenience */
