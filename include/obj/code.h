#ifndef rascal_code_h
#define rascal_code_h

#include "obj.h"
#include "obj/envt.h"

typedef struct cons_t  cons_t;
typedef struct code_t  code_t;
typedef struct func_t  func_t;
typedef struct multi_t multi_t;

typedef struct instr_t
{
  arity_t   len;
  arity_t   cap;
  opcode_t *instr;
} instr_t;

typedef struct code_t
{
  
  arity_t n_stack;
  arity_t n_cap;

  func_t  *func;
  envt_t  *envt;
  instr_t *code;
  vals_t  *vals;
} code_t;

#endif
