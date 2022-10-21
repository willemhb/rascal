#ifndef rascal_obj_cvalue_h
#define rascal_obj_cvalue_h

#include "obj/object.h"

// immediate types and object types that represent primitive atomic values
typedef struct rl_sint64_t
{
  OBJECT
  sint64_t data;
} rl_sint64_t;

typedef struct rl_uint64_t
{
  OBJECT
  uint64_t data;
} rl_uint64_t;

typedef struct cstring_utf8_t
{
  OBJECT
  utf8_t   *data;
} cstring_utf8_t;

typedef struct cstring_ascii_t
{
  OBJECT
  ascii_t *data;
} cstring_ascii_t;

typedef struct cstring_latin1_t
{
  OBJECT
  latin1_t *data;
} cstring_latin1_t;

typedef struct cstring_utf16_t
{
  OBJECT
  utf16_t *data;
} cstring_utf16_t;

typedef struct cstring_utf32_t
{
  OBJECT
  utf32_t *data;
} cstring_utf32_t;

// globals
// primitive numeric and character types
extern type_t SInt8Type,  UInt8Type, AsciiType, UTF8Type, Latin1Type;
extern type_t SInt16Type, UInt16Type, UTF16Type;
extern type_t SInt32Type, UInt32Type, UTF32Type, Real32Type;
extern type_t SInt64Type, UInt64Type, Real64Type;

// other primitive immediates
extern type_t BooleanType, OpcodeType, PrimitiveType;

// special numeric types
extern type_t FixnumType, RealType, PointerType;

// wrapper types for C strings (distinct from rascal strings)
// these exist mostly for wrapping ffi values and representing unboxed strings in builtin objects
extern type_t CStringU8Type, CStringU16Type, CStringU32Type, CStringAsciiType, CStringLatin1Type, CString;

// array of immediate types (for looking up type by code)
extern type_t *ImmTypes[N_IMM];

// convenience

#endif
