#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "memory.h"



/* globals */
#define N_HEAP (UINT16_MAX) * sizeof(List)
#define HEAP_PRESSURE 0.75
#define HEAP_RESIZE   2.00

struct Vec Grays = {
  .obj={
    .next=NULL,
    .type=VEC_OBJ,
    .flags=STATIC_OBJ,
    .gray=false,
    .black=true
  },
  .array=NULL,
  .count=0,
  .cap  =CAP_MIN
};

struct Heap Heap = {
  .allocated=0,
  .available=N_HEAP,
  .live     =&Grays.obj,
  .grays    =&Grays
};

/* API */
void collect_garbage(void) {
  Heap.available *= HEAP_RESIZE;
}

bool  check_gc(usize n_bytes) {
  return Heap.allocated+n_bytes > Heap.available;
}

void *allocate(usize n_bytes) {
  if (check_gc(n_bytes))
    collect_garbage();

  void *out = malloc(n_bytes);

  if (out == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }

  memset(out, 0, n_bytes);
  Heap.allocated += n_bytes;

  return out;
}

void *reallocate(void *ptr, usize new_n_bytes, usize old_n_bytes) {
  if (new_n_bytes > old_n_bytes) {
    if (check_gc(new_n_bytes - old_n_bytes))
      collect_garbage();

    Heap.allocated += new_n_bytes - old_n_bytes;
  } else {
    Heap.allocated -= old_n_bytes - new_n_bytes;
  }

  void *out = realloc(ptr, new_n_bytes);

  if (out == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
    

  if (new_n_bytes > old_n_bytes)
    memset(out+old_n_bytes, 0, new_n_bytes-old_n_bytes);

  return out;
}

void *duplicate(void *ptr, usize n_bytes) {
  assert(n_bytes > 0);
  
  void *copy = allocate(n_bytes);

  memcpy(copy, ptr, n_bytes);
  return copy;
}

void deallocate(void *ptr, usize n_bytes) {
  free(ptr);
  Heap.allocated -= n_bytes;
}
