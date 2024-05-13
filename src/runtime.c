#include "runtime.h"


/* Error APIs */
rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...);

/* HeapState API */
// preserving values
void unpreserve(HeapFrame* frame) {
  Heap.fp = frame->next;
}

// memory management
rl_status_t allocate(void** buf, size_t n_bytes, bool use_heap);
rl_status_t reallocate(void** buf, size_t old_n_bytes, size_t new_n_bytes, bool use_heap);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);
rl_status_t collect_garbage(void);

// lifetime
rl_status_t init_heap_state(HeapState* heap_state);
rl_status_t mark_heap_state(HeapState* heap_state);
rl_status_t free_heap_state(HeapState* heap_state);
