#ifndef rl_util_unicode_h
#define rl_util_unicode_h

#include <string.h>

#include "common.h"

/* unicode and string utilities. */

/* C types */
typedef enum {
  BINARY, ASCII, LATIN1, UTF8, UTF16, UTF32
} Encoding;

typedef enum {
  /* void type */
  VOID,
  /* unsigned integer types */
  UINT8, UINT16, UINT32, UINT64,
  /* signed integer types */
  SINT8, SINT16, SINT32, SINT64,
  /* floating point types */
  FLT32, FLT64,
  /* pointer types */
  ANYPTR, FUNPTR
} CType;

/* External API */
bool   is_mb(Encoding enc);
size_t enc_nbytes(Encoding enc);
CType  enc_Ctype(Encoding enc);
size_t Ctype_nbytes(CType Ct);

#endif
