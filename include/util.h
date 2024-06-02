#ifndef rl_util_h
#define rl_util_h

#include "common.h"

/* miscellaneous utilities go here. */

/* C types */

/* Encodes a value's machine type */

typedef enum {
  VOID     =0x00,

  UNSIGNED =0x00,
  SIGNED   =0x80,
  FLOAT    =0x40,
  ENCODED  =0xc0,
  POINTER  =0x20,
  MULTIBYYE=0x10,
  
  SIZE0    =0x00,
  SIZE8    =0x01,
  SIZE16   =0x02,
  SIZE32   =0x03,
  SIZE64   =0x04,
  SIZE128 =0x05,
  SIZE256 =0x06,
  SIZE512 =0x07,

  UINT8   =UNSIGNED|SIZE8,
  SINT8   =SIGNED|SIZE8,
  ASCII  =0x03,
  LATIN1 =0x04,
  UTF8   =0x05,

  SINT16 =0x06,
  UINT16 =0x07,
  FLOAT16=0x08,
  UTF16  =0x09,

  SINT32 =0x0a,
  UINT32 =0x0b,
  FLOAT32=0x0c,
  UTF32  =0x0d,
  
  SINT64 =0x0e,
  UINT64 =0x0f,
  FLOAT64=0x10,

  
} BinaryType;

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
