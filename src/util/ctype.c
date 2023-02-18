#include "util/ctype.h"

/* globals */
char* ShortCtypeNames[NUM_CTYPES] = {
    "v",
    "i8",  "u8",  "ascii", "latin1", "utf8",
    "i16", "u16", "utf16",
    "i32", "u32", "utf32", "f32", "p32",
    "i64", "u64", "f64", "p64"
  };

char* LongCtypeNames[NUM_CTYPES] = {
    "void",
    "int8", "uint8", "ascii", "latin1", "utf8",
    "int16", "uint16", "utf16",
    "int32", "uint32", "utf32", "float32", "ptr32",
    "int64", "uint64", "float64", "ptr64"
  };

/* API */
usize ctype_size(Ctype type) {
  if (type == 0)
    return 0;

  if (type < SINT16)
    return 1;

  if (type < SINT32)
    return 2;

  if (type < SINT64)
    return 4;

  return 8;
}

bool is_encoding(Ctype type) {
  static bool jumptable[NUM_CTYPES] = {
    [ASCII] = true, [LATIN1] = true, [UTF8] = true, [UTF16] = true, [UTF32] = true
  };

  return jumptable[type];
}

bool is_signed(Ctype type) {
  static bool jumptable[NUM_CTYPES] = {
    [SINT8] = true, [SINT16] = true, [SINT32] = true, [SINT64] = true
  };

  return jumptable[type];
}

bool is_unsigned(Ctype type) {
  static bool jumptable[NUM_CTYPES] = {
    [UINT8] = true, [UINT16] = true, [UINT32] = true, [UINT64] = true
  };

  return jumptable[type];
}

bool is_float(Ctype type) {
  static bool jumptable[NUM_CTYPES] = {
    [FLOAT32] = true, [FLOAT64] = true
  };

  return jumptable[type];
}

bool is_pointer(Ctype type) {
  static bool jumptable[NUM_CTYPES] = {
    [PTR32] = true, [PTR64] = true
  };

  return jumptable[type];
}
