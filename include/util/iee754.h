#ifndef rascal_util_ieee754_h
#define rascal_util_ieee754_h

#include "common.h"

// C types
typedef union {
  double    number;
  word_t    word;
  struct {
    uintptr_t frac : 52;
    uintptr_t expt : 11;
    uintptr_t sign :  1;
  } bits;
} IEEE_754_64;

// external APIs
double word_to_double(uintptr_t word);


#endif
