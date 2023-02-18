#ifndef util_ctype_h
#define util_ctype_h

#include "common.h"

/* C types */
typedef enum Ctype Ctype;

enum Ctype {
  VOID   =0b00000,
  SINT8  =0b00001,
  UINT8  =0b00010,
  ASCII  =0b00011,
  LATIN1 =0b00100,
  UTF8   =0b00101,
  SINT16 =0b00110,
  UINT16 =0b00111,
  UTF16  =0b01000,
  SINT32 =0b01001,
  UINT32 =0b01010,
  UTF32  =0b01011,
  FLOAT32=0b01100,
  PTR32  =0b01101,
  SINT64 =0b01110,
  UINT64 =0b01111,
  FLOAT64=0b10000,
  PTR64  =0b10001
};

/* globals */
#define NUM_CTYPES (PTR64+1)

/* API */
usize ctype_size(Ctype type);
bool is_encoding(Ctype type);
bool is_signed(Ctype type);
bool is_unsigned(Ctype type);
bool is_float(Ctype type);
bool is_pointer(Ctype type);

#endif
