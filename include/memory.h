#ifndef rascal_memory_h
#define rascal_memory_h

#include "core.h"

typedef enum {
  mem_traversed = 0x01,
  mem_finalize  = 0x02
} memflags_t;

// exports --------------------------------------------------------------------
uint_t  getmemfl(void_t *ptr);
uint_t  setmemfl(void_t *ptr);

size_t  align(size_t nbytes, size_t alignment);
bool_t  within(void_t *p, void_t *base, void_t *end);
void_t *allocate(size_t nbytes, bool_t global);

void    manage(void); // GC entry point

#endif
