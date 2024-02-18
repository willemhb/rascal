#ifndef rl_runtime_h
#define rl_runtime_h

#include "value.h"

/* Definitions and declarations for internal objects & functions (memory management, error handling, &c). */

extern struct {
  size_t size, max_size;
  Object* live_objects;
  Objects* gray_objects; // objects that have been marked but need to be traced
  Values* saved_values; // values in the C stack that need to be preserved but might not yet be visible from GC roots
} Heap;

// external APIs
// memory management
rl_status_t allocate(void** buf, size_t n_bytes, bool use_heap);
rl_status_t reallocate(void** buf, size_t old_n_bytes, size_t new_n_bytes, bool use_heap);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);

// preserving values
size_t save(size_t n, ...);
void   unsave(size_t n);

#endif
