#include <string.h>


#include "util/string.h"

/* commentary */


/* C types */


/* globals */


/* API */
size_t strlen8( const char *s ) { return strlen(s); }

size_t strlen16( const utf16_t *s )
{
  size_t n;

  for (n=0; !*s; s++) n++;

  return n;
}

size_t strlen32( const utf32_t *s )
{
  size_t n;

  for (n=0; !*s; s++) n++;

  return n;
}

int memcmp8( const byte *a, const byte *b, size_t max_cmp )
{
  return memcmp(a, b, max_cmp);
}

int memcmp16( const ushort *a, const ushort *b, size_t max_cmp )
{
  uint out = 0;

  while ( max_cmp-- )
      if ((out=*(a++) - *(b++)))
	break;

  return out;
}

int memcmp32( const uint *a, const uint *b, size_t max_cmp )
{
  uint out = 0;

  while ( max_cmp-- )
    if ((out=*(a++) - *(b++)))
      break;

  return out;
}

long memcmp64( const ulong *a, const ulong *b, size_t max_cmp )
{
  ulong out = 0;

  while ( max_cmp-- )
    if ((out=*(a++) - *(b++)))
      break;

  return out;
}

/* runtime */

/* convenience */
