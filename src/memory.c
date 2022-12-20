#include <stdlib.h>

#include "memory.h"

#include "util/memory.h"


void *alloc(size_t n) {
  return malloc_s(n);
}

void *alloc_array(size_t size, size_t obsize) {
  return calloc_s(size, obsize);
}

void *alloc_table(size_t base, size_t size, size_t obsize) {
  return alloc(base+size*obsize);
}

void *adjust(void *ptr, size_t oldsize, size_t newsize) {
  (void)oldsize;

  return realloc_s(ptr, newsize);
}

void *adjust_array(void *ptr, size_t oldsize, size_t newsize, size_t obsize) {
  (void)oldsize;

  return realloc_s(ptr, newsize*obsize);
}

void *adjust_table(void *ptr, size_t base, size_t oldsize, size_t newsize, size_t obsize) {
  return adjust(ptr, base+oldsize*obsize, base+newsize*obsize);
}

void dealloc(void *spc, size_t size) {
  (void)size;

  free(spc);
}

void dealloc_array(void *spc, size_t size, size_t obsize) {
  (void)size;
  (void)obsize;
  free(spc);
}

void dealloc_table(void *spc, size_t base, size_t size, size_t obsize) {
  (void)base;
  (void)size;
  (void)obsize;
  free(spc);
}
