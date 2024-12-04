#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Various enum types */
typedef enum Type : uint8 {
  // immediate types
  T_NUL    =  1,
  T_BOOL   =  2,
  T_GLYPH  =  3,
  T_NUM    =  4,
  T_PTR    =  5,

  // callable types
  T_PRIMFN =  6,
  T_USERFN =  7,

  // reference types
  T_SYM    =  8,

  // IO types
  T_PORT   =  9,
  T_STR    = 10,

  // user collection types
  T_LIST   = 11,
  T_PAIR   = 12,
  T_BIN    = 13,
  T_VEC    = 14,
  T_MAP    = 15,

  // internal collection types
  T_BUFFER = 16,
  T_ALIST  = 17,
  T_TABLE  = 18,

  // internal node types
  T_VNODE  = 19,
  T_MNODE  = 20,

  // miscellaneous internal types
  T_SEQ    = 21,
  T_NS     = 22,
  T_ENV    = 23,
  T_REF    = 24,
  T_UPV    = 25,
} Type;

typedef enum Error {
  E_OKAY    = 0,
  E_USER    = 1,
  E_READ    = 2,
  E_COMP    = 3,
  E_EVAL    = 4,
  E_RUNT    = 5,
  E_SYS     = 6,
} Error;

typedef enum MFlags : uint8 {
  MF_SEALED     =   1,
  MF_NOHASH     =   2,
  MF_NOTRACE    =   4,
  MF_NOSWEEP    =   8,
  MF_NOFREE     =  16,
  MF_RESERVED   =  32,
  MF_GRAY       =  64,
  MF_BLACK      = 128,
  } MFlags;

typedef enum Opcode : sint16 {
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

  // error handling instructiosn
  O_CATCH   = 19,
  O_ECATCH  = 20,
  O_THROW   = 21,

  // function calls and returns
  O_CLOSURE = 22,
  O_CALL    = 23,
  O_RTN     = 24,
} Opcode;

typedef enum RefType : uint8 {
  R_LOCAL     = 1, // local variable reference
  R_UPVAL     = 2, // captured local variable reference
  R_NAMESPACE = 3, // namespace variable reference
} RefType;

#define N_TYPES   (T_UPV+1)
#define N_OPCODES (O_RTN+1)
#define N_ERRORS  (E_SYSTEM+1)

#endif
