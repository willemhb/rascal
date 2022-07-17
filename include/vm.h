#ifndef rascal_vm_h
#define rascal_vm_h

#include "rascal.h"

typedef enum {
  /* 0-argument codes */
  op_return,

  /* 1-argument opcodes */
  op_loadc,

  op_loadg, op_storeg,

  op_call,

  num_opcodes
} opcode_t;

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
void builtin_ord( size_t n );
void builtin_not( size_t n );
void builtin_boolp( size_t n );
void builtin_nilp( size_t n );

void builtin_symbolp( size_t n );
void builtin_boundp( size_t n );

value_t compile( value_t form );
value_t execute( value_t code );

#endif
