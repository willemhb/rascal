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

   // function calls
   CALL    =0x05,
   DISPATCH=0x06,
  };

// vm state type
struct vm_t
{
  object_t    obj;
  stack_t *stack;
  code_t  *code;
  op_t    *ip;
};

// and methods
void init_vm( vm_t *vm );
void trace_vm( object_t *obj );
void free_vm( object_t *obj );

// globals
extern vm_t Vm;

// forward declarations
value_t lisp_exec( vm_t *vm, code_t *code );
void  lisp_repl( void );
void  exec_init( void );

#endif
