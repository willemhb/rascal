#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Various enum types */
typedef enum VType {
  // data types
  T_UNIT   =  1,
  T_BOOL   =  2,
  T_GLYPH  =  3,
  T_NUM    =  4,
  T_PTR    =  5,
  T_FUNC   =  6,
  T_CHUNK  =  7,
  T_PRIM   =  8,
  T_MTROOT =  9,
  T_MTNODE = 10,
  T_MTLEAF = 11,
  T_SYM    = 12,
  T_TYPE   = 13,
  T_PORT   = 14,
  T_STR    = 15,
  T_BIN    = 16,
  T_LIST   = 17,
  T_VEC    = 18,
  T_MAP    = 19,
  T_MSTR   = 20,
  T_MBIN   = 21,
  T_MVEC   = 22,
  T_MMAP   = 23,
  T_VNODE  = 24,
  T_VLEAF  = 25,
  T_MNODE  = 26,
  T_MLEAF  = 27,
  T_MLEAFS = 28,
  T_RT     = 29,
  T_CNTL   = 30,
  T_NS     = 31,
  T_ENV    = 32,
  T_REF    = 33,
  T_UPV    = 34,
} VType;

// Not VTypes but the ID needs to be known
#define T_NONE 35
#define T_ANY  36


typedef enum Kind {
  K_BOTTOM =  0,
  K_VALUE  =  1,
  K_TOP    =  2,
} Kind;

typedef enum Error {
  E_OKAY    = 0,
  E_USER    = 1,
  E_READ    = 2,
  E_COMP    = 3,
  E_EVAL    = 4,
  E_RUNT    = 5,
  E_SYS     = 6,
} Error;

typedef enum MFlags {
  MF_SEALED     =   1,
  MF_NOHASH     =   2,
  MF_NOTRACE    =   4,
  MF_NOSWEEP    =   8,
  MF_NOFREE     =  16,
  MF_RESERVED   =  32,
  MF_GRAY       =  64,
  MF_BLACK      = 128,
  } MFlags;

typedef enum Opcode {
  // stack manipulation
  O_NOOP    =  0,
  O_POP     =  1,

  // value load/store
  O_LDT     =  2,
  O_LDF     =  3,
  O_LDN     =  4,
  O_LDVAL   =  5,

  // variable load/store
  O_LDSTK   =  6,
  O_PUTSTK  =  7,
  O_LDUPV   =  8,
  O_PUTUPV  =  9,
  O_LDNS    = 10,
  O_PUTNS   = 11,
  O_LDGL    = 12,
  O_PUTGL   = 13,
  O_LDQNS   = 13,
  O_PUTQNS  = 14,

  // jump instructions
  O_JMP     = 16,
  O_JMPF    = 17,
  O_JMPT    = 18,

  // error handling instructions
  O_CATCH   = 19,
  O_ECATCH  = 20,
  O_THROW   = 21,

  // function calls and returns
  O_CLOSURE = 22,
  O_CALL    = 23,
  O_RTN     = 24,
} Opcode;

typedef enum RefType {
  R_LOCAL     = 1, // local variable reference
  R_UPVAL     = 2, // captured local variable reference
  R_NAMESPACE = 3, // namespace variable reference
} RefType;

#define N_VTYPES   (T_UPV+1)
#define N_TYPES    (T_ANY+1)
#define N_KINDS    (K_TOP+1)
#define N_OPCODES  (O_RTN+1)
#define N_ERRORS   (E_SYSTEM+1)
#define N_REFTYPES (R_NAMESPACE+1)

#endif
