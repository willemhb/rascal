#ifndef rascal_number_h
#define rascal_number_h

#include "obj/cvalue.h"
#include "obj/record.h"

// big number types
struct ratio_t
{
  record_t record;
  value_t  numer;
  value_t  denom;
};

struct complex_t
{
  record_t record;
  real_t   real;
  real_t   imag;
};

struct rl_int64_t
{
  cvalue_t cvalue;
  int64_t  value;
};

struct rl_uint64_t
{
  CVAL
  uint64_t value;
};

struct bigint_t
{
  CVAL
  arity_t  length;
  int      sign;
  byte     digits[0];
};

// forward declarations


// convenience


#endif
