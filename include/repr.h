#ifndef rl_repr_h
#define rl_repr_h

#include "rascal.h"

/* A repr is a C type that the interpreter knows how to handle. It is distinct from a
   value's rascal type. */

typedef enum repr_t
  {
    repr_void,

    /* character reprs */
    repr_ascii, repr_latin1, repr_utf8, repr_utf16, repr_utf32,

    /* unsigned reprs */
    repr_uint8, repr_uint16, repr_uint32, repr_uint64,

    /* signed reprs */
    repr_sint8, repr_sint16, repr_sint32, repr_sint64,

    /* floating point reprs */
    repr_flo32, repr_flo64,

    /* lisp value reprs */
    repr_val, repr_obj, repr_vals, repr_objs,

    /* pointer type reprs */
    repr_vptr, repr_fptr, repr_bptr, repr_sptr,

    /* string type reprs */
    repr_astr, repr_l1str, repr_u8str, repr_u16str, repr_u32str,
  } repr_t;

typedef struct
{
  char   *name;
  repr_t  repr;
  repr_t  numtype;
  size_t  size;
  size_t  align;
} Ctype_t;

static Ctype_t Reprs[] =
  {
    [repr_void] = { "void", repr_void, repr_void, sizeof(void), alignof(void) },
    [repr_ascii] = { "ascii", repr_ascii, repr_uint8, sizeof(ascii_t), alignof(ascii_t) },
    [repr_latin1] = { "latin1", repr_latin1, repr_uint8, sizeof(latin1_t), alignof(latin1_t) },
    [repr_utf8] = { "utf8", repr_utf8, repr_uint8, sizeof(utf8_t), alignof(utf8_t) },
    [repr_utf16] = { "utf16", repr_utf16, repr_uint16, sizeof(utf16_t), alignof(utf16_t) },
    [repr_utf32] = { "utf32", repr_utf32, repr_uint32, sizeof(utf32_t), alignof(utf32_t) },
    [repr_uint8] = { "uint8", repr_uint8, repr_uint8, sizeof(uint8_t), alignof(uint8_t) },
    [repr_uint16] = { "uint16", repr_uint16, repr_uint16, sizeof(uint16_t), alignof(uint16_t) },
    [repr_uint32] = { "uint32", repr_uint32, repr_uint32, sizeof(uint32_t), alignof(uint32_t) },
    [repr_uint64] = { "uint64", repr_uint64, repr_uint64, sizeof(uint64_t), alignof(uint64_t) },
    [repr_sint8] = { "sint8", repr_sint8, repr_sint8, sizeof(int8_t), alignof(int8_t) },
    [repr_sint16]={ "sint16", repr_sint16, repr_sint16, sizeof(int16_t), alignof(int16_t) },
    [repr_sint32]={ "sint32", repr_sint32, repr_sint32, sizeof(int32_t), alignof(int32_t) },
    [repr_sint64]={ "sint64", repr_sint64, repr_sint64, sizeof(int64_t), alignof(int64_t) },
    [repr_flo32] ={ "float32", repr_flo32,  repr_flo32,  sizeof(float), alignof(float) },
    [repr_flo64] ={ "float64", repr_flo64,  repr_flo64,  sizeof(double), alignof(double) },
    [repr_val] = { "rl-value", repr_val, repr_uint64, sizeof(value_t), alignof(value_t) },
    [repr_obj] = { "rl-object", repr_obj, repr_uint64, sizeof(object_t*), alignof(object_t*) },
    [repr_vals] = { "rl-value-array", repr_vals, repr_uint64, sizeof(value_t*), alignof(value_t*) },
    [repr_objs] = { "rl-object-array", repr_objs, repr_uint64, sizeof(object_t**), alignof(object_t**) },
    [repr_vptr]  ={ "void-ptr", repr_vptr,   repr_uint64, sizeof(void*),    alignof(void*) },
    [repr_fptr]  ={ "func-ptr", repr_fptr,   repr_uint64, sizeof(funcptr),  alignof(funcptr) },
    [repr_bptr]  ={ "byte-ptr", repr_bptr,   repr_uint64, sizeof(byte*),    alignof(byte*) },
    [repr_sptr] = { "file-ptr", repr_sptr, repr_uint64, sizeof(FILE*),    alignof(FILE*) },
    [repr_astr] = { "ascii-str", repr_astr, repr_uint64, sizeof(ascii_t*), alignof(ascii_t*) },
    [repr_l1str] = { "latin1-str", repr_l1str, repr_uint64, sizeof(latin1_t*), alignof(latin1_t*) },
    [repr_u8str] = { "utf8-str", repr_u8str, repr_uint64, sizeof(utf8_t*), alignof(utf8_t*) },
    [repr_u16str] = { "utf16-str", repr_u16str, repr_uint64, sizeof(utf16_t*), alignof(utf16_t*) },
    [repr_u32str] = { "utf32-str", repr_u32str, repr_uint64, sizeof(utf32_t*), alignof(utf32_t*) }
  };

#endif
