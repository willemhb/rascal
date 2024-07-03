#ifndef rl_vm_heap_h
#define rl_vm_heap_h

#include "val/state.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */

/* Globals */
/* HState & Heap APIs */
// preserving values
void unpreserve(HFrame* f);

#define preserve(p, n, vals...)                                         \
  Val __heap_frame_vals__[(n)] = { vals };                              \
  HFrame __heap_frame__ cleanup(unpreserve) =                           \
    { .proc=(p),                                                        \
      .next=(p)->hfs,                                                   \
      .count=(n),                                                       \
      .values=__heap_frame_vals__ };                                    \
  (p)->hfs=&__heap_frame__

// memory management
void*    allocate(Proc* p, size_t n);
void*    reallocate(Proc* p, void* s, size_t o, size_t n);
void*    duplicate(Proc* p, void* s, size_t n);
void     deallocate(Proc* p, void* s, size_t n);
void     collect_garbage(Proc* p, size_t n);
void     add_to_heap(Proc* p, void* o);
void     push_gray(Proc* p, void* o);
Obj*     pop_gray(Proc* p);

#endif
