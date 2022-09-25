#ifndef rascal_utils_str_h
#define rascal_utils_str_h

#include "core.h"
#include "utils/num.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    enc_ascii =16|Ctype_sint8,
    enc_latin1=16|Ctype_uint8,
    enc_utf8  =32|Ctype_sint8,
    enc_utf16 =16|Ctype_sint16,
    enc_utf32 =16|Ctype_sint32
  } encoding_t;

// character encoding utilities -----------------------------------------------
bool    is_multibyte( encoding_t enc );
Ctype_t enc_Ctype( encoding_t enc );
size_t  enc_size( encoding_t enc );

// string and bytes hashing utilities -----------------------------------------
hash_t hash_string( char *chars );
hash_t hash_bytes( byte_t *mem, arity_t cnt );

#endif
