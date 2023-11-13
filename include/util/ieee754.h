#ifndef rl_util_ieee754_h
#define rl_util_ieee754_h

#include "common.h"

/* C types */
typedef union {
  double    dbl;
  word_t    word;
  struct {
    uintptr_t frac : 52;
    uintptr_t expt : 11;
    uintptr_t sign :  1;
  } bits;
} ieee_64_t;

/* external APIs */
double word_to_double(word_t word);
word_t double_to_word(double dbl);

#endif
