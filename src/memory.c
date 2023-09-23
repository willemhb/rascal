#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "object.h"
#include "memory.h"

// internal GC helpers
static void mark(void)   {}
static void trace(void)  {}
static void sweep(void)  {}
static void resize(void) { vm.heap.heapCapacity <<= 1; }

static void manage(void) {
  mark();
  trace();
  sweep();
  resize();
}

static void manageHeap(size_t nBytesAdded, size_t nBytesRemoved) {
  if (nBytesAdded > nBytesRemoved) {
    size_t diff = nBytesAdded - nBytesRemoved;

    if (diff + vm.heap.heapUsed > vm.heap.heapCapacity)
      manage();

    vm.heap.heapUsed += diff;
  } else {
    size_t diff = nBytesRemoved - nBytesAdded;
    vm.heap.heapUsed -= diff;
  }
}

// heap/memory API
#define N_HEAP (UINT16_COUNT * sizeof(Value))

void initHeap(Heap* heap) {
  heap->objects      = NULL;
  heap->heapUsed     = 0;
  heap->heapCapacity = N_HEAP;
  initObjects(&heap->grays);
}

void freeHeap(Heap* heap) {
  freeObjects(&heap->grays);

  Obj* obj = heap->objects,* tmp;

  while (obj != NULL) {
    tmp = obj;
    obj = obj->next;
    
    freeObject(tmp);
  }
}

void* allocate(size_t nBytes, bool fromHeap) {
  if (fromHeap)
    manageHeap(nBytes, 0);
  
  void* out = SAFE_MALLOC(nBytes);
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
  
  else {
    if (fromHeap)
      manageHeap(newSize, oldSize);

    out = SAFE_REALLOC(pointer, newSize);

    if (newSize > oldSize)
      memset(out+oldSize, 0, newSize-oldSize);
  }
  
  return out;
}

void deallocate(void* pointer, size_t nBytes, bool fromHeap) {
  if (fromHeap)
    manageHeap(0, nBytes);

  free(pointer);
}
