#include "exec.h"
#include "read.h"
#include "prin.h"
#include "array.h"
#include "code.h"
#include "envt.h"

// vm implementation
void init_vm(vm_t *vm )
{
  init_obj( &vm->obj, VM, 0 );
  vm->stack = new_stack();
  vm->code  = NULL;
  vm->ip    = NULL;
}

void trace_vm( obj_t *obj )
{
  vm_t *vm = (vm_t*)obj;
  mark_obj( (obj_t*)vm->stack );
  mark_obj( (obj_t*)vm->code );
}

// vm entry point
val_t lisp_exec( vm_t *vm, code_t *code )
{
  static void* labels[] =
    {
     [HALT]  = &&halt,

     [LOADC] = &&loadc,

     [LOADG] = &&loadg, [STOREG] = &&storeg,

     [JUMP]  = &&jump,

     [CALL]  = &&call,
    };

  vm->code = code;
  vm->ip   = code->instr->data;

  val_t tmpx;
  op_t op; int16_t argx;

 dispatch:
  op = (*vm->ip)++;
  if (op > HALT)
    argx = (*vm->ip)++;

  goto *labels[op];

 halt:
  stack_pop( vm->stack, &tmpx );
  return tmpx;

 loadc:
  stack_push( vm->stack, code->constants->data[argx] );

  goto dispatch;

 loadg:
  stack_push( vm->stack, Toplevel.data[argx]->bind );

  goto dispatch;

 storeg:
  Toplevel.data[argx]->bind = stack_tos(vm->stack);

  goto dispatch;

 jump:
  vm->ip += argx;

  goto dispatch;
}

// REPL
#define INPROMPT  "<< "
#define OUTPROMPT ">> "

void repl( void )
{
  for (;;)
    {
      port_take(&Ins);
      printf(INPROMPT);
      val_t val = lisp_read(&Ins);
      printf("\n"OUTPROMPT);
      lisp_prin(&Outs, val);
      printf("\n");
    }
}

// initialization
void vm_init( void )
{
  init_vm(&Vm);
}
