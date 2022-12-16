#ifndef rl_util_string_h
#define rl_util_string_h

#include <string.h>

#include "common.h"

/* commentary

   Filling in gaps in C strings library, especially for handling different types of
   unicode strings.
 */

bool   streq8( const char *stringX, const char *stringY );
bool   streq16( const utf16_t *stringX, const utf16_t *stringY );
bool   streq32( const utf32_t *stringX, const utf32_t *stringY );

#define streq(x, y) _Generic((x), char*: streq8, utf16_t*: streq16, utf32_t*: streq32)((x), (y))

bool   str_contains_substr( const char *string, const char *substring );
bool   str_contains_char( const char *string, int ch );
bool   str_starts_with_substr( const char *string, const char *substring );
bool   str_starts_with_char( const char *string, int ch );
bool   str_ends_with_substr( const char *string, const char *substring );
bool   str_ends_with_char( const char *string, int ch );

#define str_contains( s, x ) _Generic((x), char*: str_contains_substr, default: str_contains_char )((s), (x))
#define str_starts_with( s, x ) _Generic((x), char*: str_starts_with_substr, default: str_starts_with_char )((s), (x))
#define str_ends_with( s, x ) _Generic((x), char*: str_ends_with_substr, default: str_ends_with_char )((s), (x))

int    strcmp8( const char *a, const char *b );
int    strcmp16( const utf16_t *a, const utf16_t *b );
int    strcmp32( const utf32_t *a, const utf32_t *b );

size_t strlen8( const char *string );
size_t strlen16( const utf16_t *u16string );
size_t strlen32( const utf32_t *u32string );

int    memcmp8( const byte *a, const byte *b, size_t max_cmp );
int    memcmp16( const ushort *a, const ushort *b, size_t max_cmp );
int    memcmp32( const uint *a, const uint *b, size_t max_cmp );
long   memcmp64( const ulong *a, const ulong *b, size_t max_cmp );

#endif
