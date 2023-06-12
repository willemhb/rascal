#ifndef opcodes_h
#define opcodes_h

#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  // 0-input instructions -----------------------------------------------------
  OP_NOOP,

  // builtins -----------------------------------------------------------------
  // constructors (match type codes) ------------------------------------------
  OP_SYMBOL, OP_CONS,  OP_BINARY, OP_VECTOR, OP_TABLE, OP_FUNCTION,
  OP_NUMBER, OP_GLYPH, OP_PORT,   OP_UNIT,

  // other constructors -------------------------------------------------------
  OP_LIST,

  // arithmetic ---------------------------------------------------------------
  OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_REM,
  OP_EQP, OP_LTP,

  // predicates ---------------------------------------------------------------
  OP_SYMBOLP,   OP_GENSYMP,  OP_KEYWORDP,
  OP_CONSP,     OP_NILP,     OP_LISTP,
  OP_BINARYP,   OP_ENCODEDP,
  OP_VECTORP,
  OP_TABLEP,    OP_IDTABLEP, OP_MAPPINGP,
  OP_RECORDP,
  OP_FUNCTIONP, OP_BUILTINP, OP_NATIVEP, OP_CLOSUREP,
  OP_NUMBERP,   OP_INTEGERP,
  OP_GLYPHP,
  OP_PORTP,
  OP_IDP,       OP_EQLP,

  // object APIs --------------------------------------------------------------
  // generic
  OP_ARITY, OP_FIRST, OP_REST, OP_NTH, OP_ASSOC,

  // cons
  OP_CAR, OP_CDR, OP_XAR, OP_XDR,

  // vector
  OP_VREF, OP_VSET, OP_VPUSH, OP_VPOP,

  // table
  OP_TREF, OP_TSET, OP_TDEL,

  // binary
  OP_BREF, OP_BSET, OP_BPUSH, OP_BPOP,

  // interpreter --------------------------------------------------------------
  OP_APPLY,

  // miscellaneous ------------------------------------------------------------
  OP_ORD, OP_HASH, OP_TYPEOF, OP_NOT,

  // internal instructions (don't correspond to a builtin function) -----------
  OP_START,   OP_POP,
  OP_LOADV,   OP_LOADGL,  OP_PUTGL,
  OP_CLOSURE, OP_CALL,    OP_RETURN,
  OP_JUMP,    OP_JUMPN,   OP_LOADCL, OP_PUTCL
} opcode_t;

#define N_OPS (OP_PUTCL+1)

static inline usize opcode_argc( opcode_t opcode ) {
  usize argcos[N_OPS] = {
    [OP_BINARY]=1, [OP_VECTOR]=1, [OP_TABLE]=1,
    [OP_LIST]  =1,

    [OP_ADD]   =1, [OP_SUB]   =1, [OP_MUL]  =1,
    [OP_DIV]   =1,

    [OP_EQP]   =1, [OP_LTP]   =1,

    [OP_APPLY] =1,

    [OP_LOADV] =1, [OP_LOADGL]=1, [OP_PUTGL]=1,
    [OP_CALL]  =1,
    [OP_JUMP]  =1, [OP_JUMPN] =1,

    [OP_LOADCL]=2, [OP_PUTCL] =2
  };
  
  return argcos[opcode];
}

#endif
