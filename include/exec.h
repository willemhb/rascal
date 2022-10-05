#ifndef rascal_exec_h
#define rascal_exec_h

#include "obj.h"

// main driver for the VM goes in here, as well as related typedefs, &c
// opcodes
enum
  {
   HALT    =0x00,

   // load/store instructions
   LOADC   =0x01,
   LOADG   =0x02,
   STOREG  =0x03,

   // control flow
   JUMP    =0x04,
   CALL    =0x05,
  };

// vm state type
struct vm_t
{
  obj_t    obj;
  stack_t *stack;
  code_t  *code;
  op_t    *ip;
};

// and methods
void init_vm( vm_t *vm );
void trace_vm( obj_t *obj );
void free_vm( obj_t *obj );

// globals
extern vm_t Vm;

// forward declarations
val_t lisp_exec( vm_t *vm, code_t *code );
void  repl( void );
void  vm_init( void );

#endif
