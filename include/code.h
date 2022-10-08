#ifndef rascal_code_h
#define rascal_code_h

#include "array.h"

// types that represent compiled code go in here
struct instr_t
{
  object_t object;
  ARRAY_SLOTS(op_t);
};

struct code_t
{
  object_t object;
  instr_t *instr;
  alist_t *constants;
};

// forward declarations
instr_t *new_instr( void );
void     init_instr( instr_t * instr );
void     free_instr( object_t *obj );
void     resize_instr(instr_t *instr, arity_t newl );
void     instr_write(instr_t *instr, op_t *src, arity_t n);
void     instr_append(instr_t *instr, arity_t n, ...);

code_t  *new_code( void );
void     init_code( code_t *code );
void     trace_code( object_t *obj );
void     free_code( object_t *obj );

void     disassemble( stream_t *port, code_t *code );

#endif
