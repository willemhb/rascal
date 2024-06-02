#ifndef rl_util_h
#define rl_util_h

#include "common.h"

/* miscellaneous utilities go here. */

/* C types */

/* Encodes a value's machine type */

typedef enum {
  /* void type */
  VOID     =0x00,

  /* numeric types */
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

  /* pointer types */
  VPTR     =0x10,
  OPTR     =0x11,
  FPTR     =0x12,
  APTR     =0x13,
  L1PTR    =0x14,
  U8PTR    =0x15,
  U16PTR   =0x16,
  U32PTR   =0x17,
} BinaryType;

// BinaryType utilities
size_t bin_type_size(BinaryType bt);
bool   bin_type_is_encoded(BinaryType bt);
bool   bin_type_is_multibyte(BinaryType bt);
bool   bin_type_is_unsigned(BinaryType bt);
bool   bin_type_is_signed(BinaryType bt);
bool   bin_type_is_float(BinaryType bt);
bool   bin_type_is_pointer(BinaryType bt);

// miscellaneous numeric utilities
word_t ceil2(word_t w);

// file system utilities
const char* home_dir(void);
size_t abs_path(const char* path, char* buffer, size_t buffer_size);

// string utilities
size_t strsz(const char* s);
size_t strcnt(const char* s);
bool   streq(const char* sx, const char* sy);

// collection utilities
size_t adjust_stack_size(size_t new_count, size_t max_count);
size_t adjust_alist_size(size_t old_count, size_t new_count, size_t max_count);
size_t adjust_table_size(double load_factor, size_t new_count, size_t max_count);

// hashing utilities
hash_t hash_word(word_t w);
hash_t hash_chars(const char* data);
hash_t hash_bytes(const byte_t* data, size_t count);
hash_t mix_hashes(hash_t hx, hash_t hy);

#endif
