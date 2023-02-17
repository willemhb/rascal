#ifndef util_ctype_h
#define util_ctype_h

#include "common.h"

/* C types */
typedef enum Ctype Ctype;

enum Ctype {
  VOID, // no C type
  SINT8, UINT8, ASCII, LATIN1, UTF8,
  SINT16, UINT16, UTF16,
  SINT32, UINT32, UTF32, FLOAT32,
  PTR32, SINT64, UINT64, FLOAT64, PTR64
};

/* API */
usize ctype_size(Ctype type);
bool fits_ctype(void* data, Ctype type);
bool is_encoding(Ctype type);
bool is_signed(Ctype type);
bool is_unsigned(Ctype type);
bool is_float(Ctype type);
bool is_pointer(Ctype type);

#endif
