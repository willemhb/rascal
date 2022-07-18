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
void builtin_cons( size_t n );
void builtin_consp( size_t n );
void builtin_car( size_t n );
void builtin_cdr( size_t n );

void builtin_fixnump( size_t n );
void builtin_add( size_t n );
void builtin_sub( size_t n );
void builtin_mul( size_t n );
void builtin_eqp( size_t n );
void builtin_ltp( size_t n );

void builtin_idp( size_t n );
void builtin_eqlp( size_t n );
void builtin_ord( size_t n );
void builtin_not( size_t n );
void builtin_boolp( size_t n );
void builtin_nilp( size_t n );

void builtin_symbolp( size_t n );
void builtin_boundp( size_t n );

value_t compile( value_t form );
value_t execute( value_t code );

#endif
