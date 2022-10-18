#ifndef rascal_exec_h
#define rascal_exec_h

#include "obj/function.h"
#include "obj/control.h"

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
   INVOKE  =0x06,
  };

// vm state type

#define CALLSTACK_CAP 2048
#define VALSTACK_CAP  (1<<16)

struct vm_t
{
  value_t    value;                 // transmit return values (val register)
  control_t *control;               // current call frame

  vector_t  *values;
  vector_t  *frames;

  control_t  framepool[CALLSTACK_CAP];
  value_t    valpool[VALSTACK_CAP];
};

void init_vm( vm_t *vm );
void free_vm( vm_t *vm );

void vm_roots( void );
void vm_init( void );

// forward declarations
value_t lisp_exec( vm_t *vm, bytecode_t *code );
void    lisp_repl( void );

#endif
