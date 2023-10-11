#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "object.h"
#include "memory.h"

// internal GC helpers
static void mark(Vm* vm) {
  (void)vm;
}

static void trace(Vm* vm) {
  (void)vm;
}

static void sweep(Vm* vm) {
  (void)vm; 
}

static void resize(Vm* vm) {
  vm->heap.capacity <<= 1;
}

static void manage(Vm* vm) {
  mark(vm);
  trace(vm);
  sweep(vm);
  resize(vm);
}

static void manageHeap(Vm* vm, size_t nBytesAdded, size_t nBytesRemoved) {
  if (nBytesAdded > nBytesRemoved) {
    size_t diff = nBytesAdded - nBytesRemoved;

    if (diff + heapUsed(vm) > heapCapacity(vm))
      manage(vm);

    heap(vm)->used += diff;
  } else {
    size_t diff = nBytesRemoved - nBytesAdded;
    heap(vm)->used -= diff;
  }
}

// heap/memory API
#define N_HEAP (UINT16_COUNT * sizeof(Value))

void initHeap(Heap* heap) {
  heap->objects  = NULL;
  heap->used     = 0;
  heap->capacity = N_HEAP;
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

void* allocate(Vm* vm, size_t nBytes) {
  if (vm)
    manageHeap(vm, nBytes, 0);
  
  void* out = SAFE_MALLOC(nBytes);
  memset(out, 0, nBytes);
  
  return out;
}

void* duplicate(Vm* vm, void* pointer, size_t nBytes) {
  void* cpy = allocate(vm, nBytes);
  
  memcpy(cpy, pointer, nBytes);
  
  return cpy;
}

void* reallocate(Vm* vm, void* pointer, size_t oldSize, size_t newSize) {
  void* out;
  
  if (newSize == 0) {
    deallocate(vm, pointer, oldSize);
    out = NULL;
  }
  
  else {
    if (vm)
      manageHeap(vm, newSize, oldSize);

    out = SAFE_REALLOC(pointer, newSize);

    if (newSize > oldSize)
      memset(out+oldSize, 0, newSize-oldSize);
  }
  
  return out;
}

void deallocate(Vm* vm, void* pointer, size_t nBytes) {
  if (vm)
    manageHeap(vm, 0, nBytes);

  free(pointer);
}
