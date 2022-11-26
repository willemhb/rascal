#include <stdlib.h>
#include <string.h>

#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */

/* API */
void *alloc( size_t n_bytes )
{
  void *out = alloc_s(malloc, n_bytes);

  memset(out, 0, n_bytes);

  return out;
}

void *alloc_array( size_t count, size_t ob_size )
{
  return alloc(count*ob_size);
}

void  dealloc( void *ptr, size_t n_bytes )
{
  (void)n_bytes;
  free(ptr);
}

void *adjust( void *ptr, size_t old_size, size_t new_size )
{
  (void)old_size;

  void *out = alloc_s(realloc, ptr, new_size);

  return out;
}

void *adjust_array( void *ptr, size_t old_count, size_t new_count, size_t ob_size )
{
  return adjust(ptr, old_count * ob_size, new_count * ob_size);
}

void dealloc_array( void *ptr, size_t count, size_t ob_size )
{
  dealloc(ptr, count * ob_size);
}

/* runtime */
void rl_vm_memory_init( void ) {}

/* convenience */
