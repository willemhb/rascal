#ifndef rl_util_text_h
#define rl_util_text_h

#include "common.h"
/* String, text, and binary utilities. */

/* Encodes a value's machine type. Mostly maps to C scalar types, with a few extras. */

typedef enum {
  SINT8    =0x00,
  UINT8    =0x01,
  ASCII    =0x02,
  LATIN1   =0x03,
  UTF8     =0x04,

  SINT16   =0x05,
  UINT16   =0x06,
  UTF16    =0x07,

  SINT32   =0x08,
  UINT32   =0x09,
  FLOAT32  =0x0a,
  UTF32    =0x0b,

  SINT64   =0x0c,
  UINT64   =0x0d,
  FLOAT64  =0x0e,
  POINTER  =0x0f,
} CType;

// CType utilities
size_t ct_size(CType bt);
bool   ct_is_encoded(CType bt);
bool   ct_is_multibyte(CType bt);
bool   ct_is_unsigned(CType bt);
bool   ct_is_signed(CType bt);
bool   ct_is_float(CType bt);
bool   ct_is_pointer(CType bt);

// string utilities
size_t ssz(const char* s);
size_t scnt(const char* s);
int    scmp(const char* x, const char* y);
bool   seq(const char* sx, const char* sy);

#endif
