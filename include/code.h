#ifndef rascal_code_h
#define rascal_code_h

#include "array.h"

// types that represent compiled code go in here
struct instr_t
{
  obj_t obj;
  ARRAY_SLOTS(op_t);
};

struct code_t
{
  obj_t obj;
  instr_t *instr;
  alist_t *constants;
};

// forward declarations
instr_t *new_instr( void );
void     init_instr( instr_t * instr );
void     free_instr( obj_t *obj );
void     resize_instr(instr_t *instr, arity_t newl );
void     instr_write(instr_t *instr, op_t *src, arity_t n);
void     instr_append(instr_t *instr, arity_t n, ...);

code_t  *new_code( void );
void     init_code( code_t *code );
void     trace_code( obj_t *obj );
void     free_code( obj_t *obj );

void     disassemble( port_t *port, code_t *code );

#endif
