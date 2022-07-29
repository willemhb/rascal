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

// globals --------------------------------------------------------------------
extern value_t r_quote, r_do;                     // quote, do
extern value_t r_cond, r_if, r_case, r_and, r_or; // cond, if, case, and, or
extern value_t r_fun, r_let;                      // fun, let
extern value_t r_val, r_assign;                   // val, :=

extern value_t r_else;

extern value_t r_envt, r_func;

// core -----------------------------------------------------------------------
value_t compile( value_t form );
value_t execute( value_t code );
value_t apply( size_t nargs  );
value_t eval( value_t form );

// builtins -------------------------------------------------------------------
void r_builtin(compile);
void r_builtin(execute);
void r_builtin(apply);
void r_builtin(eval);
void r_builtin(envt);
void r_builtin(func);

// initialization -------------------------------------------------------------
void vm_init( void );

#endif
