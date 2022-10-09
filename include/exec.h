#ifndef rascal_exec_h
#define rascal_exec_h

#include "array.h"

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
struct control_t
{
  object_t     object;

  control_t   *caller;
  control_t   *prompt;

  stack_t     *stack;
  bytecode_t  *bytecode;
  vector_t    *upvals;
  opcode_t    *ip;

  size_t       base;
  size_t       size;
};

#define CALLSTACK_CAP 2048

struct vm_t
{
  stack_t    stack;                 // shared values stack
  value_t    value;                 // transmit return values
  control_t *control;               // current call frame
  cons_t    *open_upvals;           // open upvalues (like it says)
  size_t     frames_used;           // next available frame
  control_t  frames[CALLSTACK_CAP]; // array of active frames
};

// and methods
void init_vm( vm_t *vm );
void trace_vm( object_t *obj );
void free_vm( object_t *obj );

// globals
extern vm_t Vm;

extern type_t *VmType, *ControlType;

// forward declarations
value_t lisp_exec( vm_t *vm, bytecode_t *code );
void    lisp_repl( void );
void    exec_init( void );

#endif
