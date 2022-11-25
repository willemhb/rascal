#ifndef rl_util_string_h
#define rl_util_string_h

#include "common.h"

/* commentary

   Filling in gaps in C strings library, especially for handling different types of
   unicode strings.
 */

size_t strlen8( const char *string );
size_t strlen16( const utf16_t *u16string );
size_t strlen32( const utf32_t *u32string );

#endif
