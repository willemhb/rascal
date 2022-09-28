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

#define enc_mask 0x3f

// string and bytes hashing utilities -----------------------------------------
hash_t hash_string( char *chars );
int    u32cmp( uint32_t *xb, uint32_t *yb, size_t n );
hash_t hash_wbytes( uint32_t *wchrs, arity_t cnt );
hash_t hash_bytes( byte_t *mem, arity_t cnt );

bool  ihash_bytes( byte_t **mem, hash_t **buf, arity_t *cnt, arity_t *cap );
bool  ihash_wbytes( uint32_t **wchrs, hash_t **buf, arity_t *cnt, arity_t *cap );

// convenience
static inline size_t sizeof_enc(encoding_t enc)
{
  return sizeof_Ctype(enc&15);
}

static inline bool is_multibyte( encoding_t enc )
{
  return enc == enc_utf8 || enc == enc_utf16;
}

static inline Ctype_t enc_Ctype( encoding_t enc )
{
  return enc&15;
}

#endif
