#include <assert.h>
#include <string.h>

#include "strutils.h"
#include "numutils.h"

inline size_t strsize( char *s ) {
  return (size_t)(strlen(s) + 1);
}


size_t charsize( encoding_t enc ) {
  switch (enc) {
  case enc_latin1 ... enc_utf8: return 1;
  case enc_utf16:               return 2;
  case enc_utf32:               return 4;
  }
  __builtin_unreachable();
}

inline bool multibytep( encoding_t enc ) {
  return enc == enc_utf8 || enc == enc_utf16;
}

bool basep(int base) {
  return (base == 2) | (base == 8) || (base == 10) || (base == 16);
}

bool digitbasep(char c, int base) {
    if (base < 11)
        return (c >= '0' && c < '0'+base);
    return ((c >= '0' && c <= '9') ||
            (c >= 'a' && c < 'a'+base-10) ||
            (c >= 'A' && c < 'A'+base-10));
}

char *uint2str(char *dest, size_t len, uint64_t num, int base ) {
  assert( basep( base ) );
  assert( len >= SAFE_NUMBER_BUFFER_SIZE );

  int i = len-1;
  uint64_t b = (uint64_t)base;
  char ch;
  dest[i--] = '\0';

  while (i >= 0) {
    ch = (char)(num % b);
    if (ch < 10) ch += '0';
    else         ch = ch-10+'a';

    dest[i--] = ch;
    num /= b;

    if (num == 0)
      break;
  }

  return &dest[i+1];
}


int str2int(char *str, size_t len, int64_t *res, int base) {
  assert( basep( base ) );
  
  int64_t result, place;
  int sign = 1;
  int i;
  
  place = 1; result = 0;
    for(i=len-1; i>=0; i--) {
        char digit = str[i];

	if (digit == '-') {
	  if (i > 0) return 1;
	  
	  sign = -1;
	  break;
	}

	if (digit == '+') {
	  if (i > 0) return 1;
	  
	  break;
	}

	if (!digitbasep(digit, base))
            return 1;
	
        if (digit <= '9')
            digit -= '0';
	
        else if (digit >= 'a')
            digit = digit-'a'+10;
        else if (digit >= 'A')
            digit = digit-'A'+10;
        result += digit * place;
        place *= base;
    }

    *res = result * sign;
    return 0;
}
