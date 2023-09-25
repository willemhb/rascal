#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define DEBUG_RASCAL
// #define DEBUG_SCANNER
// #define DEBUG_PARSER

#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define QNAN         0x7ff8000000000000UL
#define SIGN         0x8000000000000000UL
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)

/**
 *
 * Used for conversion from double -> uint64_t when working with NaN-boxed values.
 *
 * Notionally useful for extracting different parts of the floating point
 * representation as well.
 * 
 **/

typedef union {
  double    number;
  uintptr_t word;
  struct {
    uintptr_t frac : 52;
    uintptr_t expt : 11;
    uintptr_t sign :  1;
  } bits;
} IEEE_754_64;

static inline uintptr_t doubleToWord(double number) {
  return ((IEEE_754_64)number).word;
}

static inline double wordToDouble(uintptr_t word) {
  return ((IEEE_754_64)word).number;
}

// other C types
// VM state objects
typedef struct Heap        Heap;
typedef struct Interpreter Interpreter;
typedef struct Environment Environment;
typedef struct Scanner     Scanner;
typedef struct Parser      Parser;
typedef struct Compiler    Compiler;
typedef struct Vm          Vm;

// misc enums
typedef enum {
  BINARY,      // not encoded
  ASCII,       // ascii encoded
} Encoding;

#endif
