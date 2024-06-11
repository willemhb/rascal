#ifndef rl_util_h
#define rl_util_h

#include "common.h"

/* miscellaneous utilities go here. */

/* C types */

/* Encodes a value's machine type */

typedef enum {
  /* numeric types */
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
