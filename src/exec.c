#include "exec.h"
#include "read.h"
#include "prin.h"
#include "array.h"

#define INPROMPT  "<< "
#define OUTPROMPT ">> "

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
  
}

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
