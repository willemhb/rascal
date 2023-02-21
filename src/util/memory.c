#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/memory.h"

// API ------------------------------------------------------------------------
// initialization helpers -----------------------------------------------------
void* mem8set( void* dest, uint8 ch, usize count ) {
  return memset(dest, ch, count);
}

void* mem16set( void* dest, uint16 ch, usize count ) {
  uint16* dest16 = dest;

  for (usize i=0; i<count; i++)
    dest16[i] = ch;

  return dest;
}

void* mem32set( void* dest, uint32 ch, usize count ) {
  uint32* dest32 = dest;

  for (usize i=0; i<count; i++)
    dest32[i] = ch;

  return dest;
}

void* mem64set( void* dest, uint64 ch, usize count ) {
  uint64* dest64 = dest;

  for (usize i=0; i<count; i++)
    dest64[i] = ch;

  return dest;
}

void* memxxset( void* dest, const void* src, usize count, usize obsize ) {
  void* out = dest, * buf = dest;
  
  for (;count; count--, buf += obsize)
    memcpy(buf, src, obsize);

  return out;
}

// safe allocators ------------------------------------------------------------
void* malloc_s( usize n ) {
  return SAFE_ALLOC(malloc, n);
}

void* calloc_s( usize n, usize obsize ) {
  return SAFE_ALLOC(calloc, n, obsize);
}

void* realloc_s( void* p, usize n ) {
  return SAFE_ALLOC(realloc, p, n);
}

void* crealloc_s( void* p, usize n, usize obsize ) {
  return SAFE_ALLOC(realloc, p, n * obsize);
}
