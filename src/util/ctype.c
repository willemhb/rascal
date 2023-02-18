#include "util/ctype.h"

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
