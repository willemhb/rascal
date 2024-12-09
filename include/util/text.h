#ifndef rl_util_text_h
#define rl_util_text_h

#include <string.h>

#include "common.h"
/* String, text, and binary utilities. */

/* Encodes a value's machine type. Mostly maps to C scalar types, with a few extras. */

typedef enum CType {
  VOID     =0x00,
  SINT8    =0x01,
  UINT8    =0x02,
  ASCII    =0x03,
  LATIN1   =0x04,
  UTF8     =0x05,

  SINT16   =0x06,
  UINT16   =0x07,
  UTF16    =0x08,

  SINT32   =0x09,
  UINT32   =0x0a,
  FLOAT32  =0x0b,
  UTF32    =0x0c,

  SINT64   =0x0d,
  UINT64   =0x0e,
  FLOAT64  =0x0f,
} CType;

// CType utilities
size_t ct_size(CType bt);
bool   ct_is_encoded(CType bt);
bool   ct_is_multibyte(CType bt);
bool   ct_is_unsigned(CType bt);
bool   ct_is_signed(CType bt);
bool   ct_is_float(CType bt);

// string utilities
void    cleanup_str(char** s);

#define tmp_str cleanup(cleanup_cstr)

char*  str(char* ini, size_t n);
bool   streql(char* sx, char* sy);

#endif
