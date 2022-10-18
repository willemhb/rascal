#ifndef rascal_exec_h
#define rascal_exec_h

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

void vm_roots( void );
void vm_init( void );

// forward declarations
value_t lisp_exec( vm_t *vm, bytecode_t *code );
void    lisp_repl( void );

#endif
