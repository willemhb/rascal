#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Various enum types */
typedef enum Type : uint8 {
  // immediate types
  T_NUL      =  1,
  T_BOOL     =  2,
  T_GLYPH    =  3,
  T_NUM      =  4,
  T_PTR      =  5,

  // callable types
  T_NATIVEFN =  6,
  T_USERFN   =  7,

  // reference types
  T_SYM      =  8,

  // IO types
  T_PORT     =  9,
  T_STR      = 10,

  // user collection types
  T_LIST     = 11,
  T_BUFFER   = 12,
  T_ALIST    = 13,
  T_TABLE    = 14,

  // miscellaneous internal types
  T_NS       = 15,
  T_ENV      = 16,
  T_REF      = 17,
  T_UPV      = 18,
} Type;

typedef enum MFlags : uint8 {
  MF_NOTRACE    =   1,
  MF_NOSWEEP    =   2,
  MF_NOFREE     =   4,
  MF_PERSISTENT =   8,
  MF_TRANSIENT  =  16,
  MF_SEALED     =  32,
  MF_GRAY       =  64,
  MF_BLACK      = 128,
  } MFlags;

typedef enum Opcode : sint16 {
  O_NOOP = 0,
  O_POP  = 1,
  
} Opcode;

typedef enum RefType : uint8 {
  R_LOCAL   = 1, // local variable reference
  R_UPVAL_L = 2, // local upvalue reference
  R_UPVAL_C = 3, // captured upvalue reference
  R_MODULE  = 4, // module variable reference
  R_GLOBAL  = 5, // global variable reference
} RefType;

#define N_TYPES (T_REF+1)

#endif
