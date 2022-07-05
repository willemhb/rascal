#ifndef strutils_h
#define strutils_h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* string and unicode utilities */
#define SAFE_NUMBER_BUFFER_SIZE 68

typedef enum encoding_t {
  enc_latin1,
  enc_ascii,
  enc_utf8,
  enc_utf16,
  enc_utf32
} encoding_t;

size_t  strsize( char *s );
size_t  charsize( encoding_t e );
bool    multibytep( encoding_t e );

// integer/string conversions -------------------------------------------------
bool    basep(int base);
bool    digitbasep(char c, int base );
char   *uint2str(char *dest, size_t len, uint64_t num, int base );
int     str2int(char *src, size_t len, int64_t *res, int base );

#endif
