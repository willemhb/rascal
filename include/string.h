#ifndef rascal_string_h
#define rascal_string_h

#include "object.h"

typedef enum
  {
    str_fl_ascii =1,
    str_fl_latin1=2,
    str_fl_utf8  =3,
    str_fl_utf16 =4,
    str_fl_utf32 =5,

    str_fl_immut =8
  } str_fl_t;

struct string_t
{
  HEADER;
  char  *data;
  uint   cnt, cap;
  ulong  hash;
};

#endif
