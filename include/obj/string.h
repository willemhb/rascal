#ifndef rascal_string_h
#define rascal_string_h

#include "obj/array.h"

// rascal string type implementations go here
// 'string' types are immutable strings intended for general use.
// 'buffer' types are mutable dynamic strings intended for use as IO buffers.
// 'cstring' are simple wrappers for C strings. They exist mostly for wrapping ffi values and
// representing unboxed strings in builtin objects

// typically, rascal strings are chosen from the narrowest unicode type that can represent
// every glyph in the string in a single unit (eg, if the larget character is '\x50', utf8
// will be chosen, if the largest is '\x01ff' utf16 will be chosen, etc). Ie, rascal strings
// and can always be treated as an array of same-sized characters.

// buffer and cstring types are selected for compactness or compatibility with an input/output
// source. In general, they are only an array of characters if their encoding is fixed-width.

// ascii string types.
typedef struct ascii_string_t
{
  ARRAY( ascii_t );
} ascii_str_t;

typedef struct ascii_buffer_t
{
  ARRAY( ascii_t );
} ascii_buffer_t;

typedef struct ascii_cstring_t
{
  OBJECT;
  ascii_t *data;
} ascii_cstring_t;

// latin1 string types.
typedef struct latin1_string_t
{
  ARRAY( latin1_t );
} latin1_str_t;

typedef struct latin1_buffer_t
{
  ARRAY( latin1_t );
} latin1_buffer_t;

typedef struct latin1_cstring_t
{
  OBJECT;
  latin1_t *data;
} latin1_cstring_t;

// utf8 string types.
typedef struct utf8_string_t
{
  ARRAY( utf8_t );
} utf8_string_t;

typedef struct utf8_buffer_t
{
  ARRAY( utf8_t );
} utf8_buffer_t;

typedef struct utf8_cstring_t
{
  OBJECT;
  utf8_t   *data;
} utf8_cstring_t;

// utf16 string types.
typedef struct utf16_string_t
{
  ARRAY( utf16_t );
} utf16_str_t;

typedef struct utf16_buffer_t
{
  ARRAY( utf16_t );
} utf16_buffer_t;

typedef struct utf16_cstring_t
{
  OBJECT;
  utf16_t *data;
} utf16_cstring_t;

// utf32 string types.
typedef struct utf32_string_t
{
  ARRAY( utf32_t );
} utf32_str_t;

typedef struct utf32_buffer_t
{
  ARRAY( utf32_t );
} utf32_buffer_t;

typedef struct utf32_cstring_t
{
  OBJECT;
  utf32_t *data;
} utf32_cstring_t;

// globals
extern type_t AsciiStringType, AsciiBufferType, AscciCStringType;
extern type_t Latin1StringType, Latin1BufferType, Latin1CString;
extern type_t UTF8StringType, UTF8BufferType, UTF8CStringType;
extern type_t UTF16StringType, UTF16BufferType, UTF16CStringType;
extern type_t UTF32StringType, UTF32BufferType, UTF32CStringType;

#endif
