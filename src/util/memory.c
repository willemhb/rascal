#include <string.h>
#include <stdlib.h>

#include "util/memory.h"


/* commentary */

/* C types */

/* globals */

/* API */
void *malloc_s( size_t n_bytes )
{
  static size_t call_counter = 0;
  call_counter++;

  void *out = alloc_s( malloc, n_bytes );

  memset(out, 0, n_bytes);

  return out;
}

void *calloc_s( size_t count, size_t ob_size )
{
  void *out = alloc_s(calloc, count, ob_size);
  memset(out, 0, count*ob_size);

  return out;
}

void *oballoc_s( size_t base, size_t count, size_t ob_size )
{
  return malloc_s(base+count*ob_size);
}

void *realloc_s( void *ptr, size_t new_size )
{
  void *out = alloc_s(realloc, ptr, new_size);

  return out;
}

void *crealloc_s( void *ptr, size_t count, size_t ob_size )
{
  return realloc_s(ptr, count*ob_size);
}

void *obrealloc_s( void *ptr, size_t base, size_t count, size_t ob_size )
{
  return realloc_s(ptr, base+count*ob_size);
}
