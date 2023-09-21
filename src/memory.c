#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "memory.h"

static bool checkHeapOverflow(size_t nBytes) {
  return nBytes + vm.heapUsed > vm.heapCapacity;
}

static void manage(void) {}

void* allocate(size_t nBytes, bool fromHeap) {
  if (fromHeap && checkHeapOverflow(nBytes))
    manage();

  void* out = malloc(nBytes);

  if (out == NULL)
    exit(1);

  memset(out, 0, nBytes);

  return out;
}

void* duplicate(void* pointer, size_t nBytes, bool fromHeap) {
  void* cpy = allocate(nBytes, fromHeap);

  memcpy(cpy, pointer, nBytes);

  return cpy;
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize, bool fromHeap) {
  void* out;
  
  if (newSize == 0) {
    deallocate(pointer, oldSize, fromHeap);
    out = NULL;
  }

  else if (newSize > oldSize) {
    size_t diff = newSize - oldSize;

    if (fromHeap && checkHeapOverflow(diff))
      manage();

    out = realloc(pointer, newSize);

    if (out == NULL)
      exit(1);

    memset(out+oldSize, 0, diff);
    vm.heapUsed += diff;
  } else {
    size_t diff  = oldSize - newSize;

    out = realloc(pointer, newSize);

    if (out == NULL)
      exit(1);

    vm.heapUsed -= diff;
  }

  return out;
}

void deallocate(void* pointer, size_t nBytes, bool fromHeap) {
  if (fromHeap)
    vm.heapUsed -= nBytes;

  free(pointer);
}
