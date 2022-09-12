#ifndef rascal_number_h
#define rascal_number_h

#include "numutils.h"

#include "object.h"
#include "array.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   char_fl_escape  =16,
   char_fl_captured=32,
   char_fl_tailpos =64
  } char_fl_t;

// forward declarations -------------------------------------------------------
value_t real(double f);
value_t character(int ch);

// macros & statics -----------------------------------------------------------
#define as_character(x) asa(character_t, x, uval)
#define as_unsigned(x)  asa(uint, x, uval)
#define as_real(x)      asa(real_t, x, dval)

static inline tag_p(char, CHARACTER)

static inline bool is_real(value_t x)
{
  return (x&QNAN) != QNAN;
}

static inline bool is_integer(value_t x)
{
  if (is_real(x))
    return dval(x) == (long)dval(x);

  if (is_char(x))
    return Ctype_is_integer(cflg(x)&15);

  return false;
}

#endif
