#ifndef rascal_code_h
#define rascal_code_h

#include "obj.h"

typedef struct func_t  func_t;
typedef struct envt_t  envt_t;
typedef struct instr_t instr_t;

typedef struct instr_t
{
  arity_t   len;
  arity_t   cap;
  opcode_t *instr;
} instr_t;

typedef struct code_t
{
  OBJ_HEAD;
  arity_t n_stack;
  arity_t n_cap;

  func_t  *func;
  envt_t  *envt;
  instr_t *code;
  vals_t  *vals;
} code_t;

#endif
