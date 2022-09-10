#ifndef rascal_number_h
#define rascal_number_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef double flonum_t;
typedef long   integer_t;
typedef char   character_t;
typedef bool   boolean_t;

typedef struct
{
  /* ideally these wouldn't be tagged but this allows bigints to be used when necessary */
  HEADER;
  value_t numer;
  value_t denom;
} ratio_t;

typedef struct
{
  HEADER;
  double real;
  double imag;
} complex_t;

typedef struct
{
  HEADER;

  union
  {
    integer_t bits_64;

    struct
    {
      char sign;
      char digits[7];
    } bits_n;
  };
} bigint_t;

#endif
