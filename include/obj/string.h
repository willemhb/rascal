#ifndef rascal_string_h
#define rascal_string_h

#include "obj/array.h"

// rascal string type implementations go here
// 'string' types are immutable strings intended for general use.
// 'buffer' types are mutable dynamic strings intended for use as IO buffers.
typedef struct ascii_string_t
{
  ARRAY( ascii_t );
} ascii_str_t;

typedef struct ascii_buffer_t
{
  ARRAY( ascii_t );
} ascii_buffer_t;

typedef struct latin1_string_t
{
  ARRAY( latin1_t );
} latin1_str_t;

typedef struct latin1_buffer_t
{
  ARRAY( latin1_t );
} latin1_buffer_t;

typedef struct utf8_string_t
{
  ARRAY( utf8_t );
} utf8_str_t;

typedef struct utf8_buffer_t
{
  ARRAY( utf8_t );
} utf8_buffer_t;

typedef struct utf16_string_t
{
  ARRAY( utf16_t );
} utf16_str_t;

typedef struct utf16_buffer_t
{
  ARRAY( utf16_t );
} utf16_buffer_t;

typedef struct utf32_string_t
{
  ARRAY( utf32_t );
} utf32_str_t;

typedef struct utf32_buffer_t
{
  ARRAY( utf32_t );
} utf32_buffer_t;

// globals
extern type_t AsciiStringType, AsciiBufferType;
extern type_t Latin1StringType, Latin1BufferType;
extern type_t UTF8StringType, UTF8BufferType;
extern type_t UTF16StringType, UTF16BufferType;
extern type_t UTF32StringType, UTF32BufferType;

// 

#endif
