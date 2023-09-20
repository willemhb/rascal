#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define RASCAL_DEBUG

#define MAX_ARITY   0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2    0x0000800000000000UL
#define ARITY_WIDTH 48

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

#endif
