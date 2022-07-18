#ifndef rascal_vm_h
#define rascal_vm_h

#include "rascal.h"

typedef enum {
  /* 0-argument codes */
  op_return,

  op_pop,

  op_loadn, op_loadt, op_loadf,

  op_closure,

  /* 1-argument opcodes */
  op_loadv,

  op_loadl, op_storel,

  op_loadg, op_storeg,

  op_jmp, op_jmpf, op_jmpt,

  op_argc, op_vargc,
  
  op_capture, op_call,

  /* 2-argument opcodes */
  op_loadc, op_storec,

  num_opcodes
} opcode_t;

extern value_t r_quote, r_if, r_lambda, r_do, r_define, r_assign;

extern value_t r_else;

// builtins -------------------------------------------------------------------
value_t compile( value_t form );
value_t execute( value_t code );
value_t apply( size_t nargs  );

// initialization -------------------------------------------------------------
void init_vm( void );

#endif
