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
  T_PAIR     = 11,
  T_BUFFER   = 12,
  T_ALIST    = 13,
  T_TABLE    = 14,

  // miscellaneous internal types
  T_ENV      = 15,
  T_REF      = 16,
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

#define N_TYPES (T_REF+1)

#endif
