#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Various enum types */
typedef enum Type : uint8_t {
  // immediate types
  T_NUL      =  1,
  T_BOOL     =  2,
  T_GLYPH    =  3,
  T_NUMBER   =  4,
  T_CPOINTER =  5,

  // callable types
  T_CONTROL  =  6,
  T_NATIVEFN =  7,
  T_USERFN   =  8,

  // reference types
  T_SYMBOL   =  9,

  // IO types
  T_STREAM   = 10,
  T_BINARY   = 11,

  // user collection types
  T_CONS     = 12,
  T_VECTOR   = 13,
  T_MAP      = 14,

  // miscellaneous internal types
  // T_STATE    = 16,
} Type;

typedef enum MFlags : uint8_t {
  MF_NOTRACE    =   1,
  MF_NOSWEEP    =   2,
  MF_NOFREE     =   4,
  MF_PERSISTENT =   8,
  MF_TRANSIENT  =  16,
  MF_SEALED     =  32,
  MF_GRAY       =  64,
  MF_BLACK      = 128,
  } MFlags;

#define N_TYPES (T_MAP+1)

#endif
