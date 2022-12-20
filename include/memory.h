#ifndef rl_memory_h
#define rl_memory_h

#include "rascal.h"

/* forward declarations */
void *alloc(size_t n);
void *alloc_array(size_t size, size_t obsize);
void *alloc_table(size_t base, size_t size, size_t obsize);

void *adjust(void *spc, size_t oldsize, size_t newsize);
void *adjust_array(void *spc, size_t oldsize, size_t newsize, size_t obsize);
void *adjust_table(void *spc, size_t base, size_t oldsize, size_t newsize, size_t obsize);

void  dealloc(void *spc, size_t size);
void  dealloc_array(void *spc, size_t size, size_t obsize);
void  dealloc_table(void *spc, size_t base, size_t size, size_t obsize);


#endif
