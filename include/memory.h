#ifndef rascal_memory_h
#define rascal_memory_h

#include "common.h"

void* allocate(size_t nBytes, bool fromHeap);
void* duplicate(void* pointer, size_t nBytes, bool fromHeap);
void* reallocate(void* pointer, size_t oldSize, size_t newSize, bool fromHeap);
void* deallocate(void* pointer, size_t nBytes, bool fromHeap);

#endif
