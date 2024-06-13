#ifndef rl_vm_heap_h
#define rl_vm_heap_h

#include "status.h"

#include "val/array.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */
/* C types */
/* frame types */
typedef struct HFrame HFrame;

/* state types */
typedef struct HState HState;

/* frame types */
struct HFrame {
  HFrame* next;
  size_t  count;
  Value*  values;
};

/* state types */
struct HState {
  HFrame*      fp; // Live objects in the C stack that may not be visible from the roots (eg, when an intermediate object is created inside a C function).
  size_t       size, max_size;
  Object*      live_objects;
  Alist*       grays;
};

/* Global State objects */
extern HState Heap;

/* HState & Heap APIs */
// preserving values
void unpreserve(HFrame* frame);

#define preserve(n, vals...)                                    \
  Value __heap_frame_vals__[(n)] = { vals };                    \
  HFrame __heap_frame__ cleanup(unpreserve) =                   \
    { .next=Heap.fp, .count=(n), .values=__heap_frame_vals__ }; \
  Heap.fp=&__heap_frame__

// memory management
void*    allocate(size_t n, bool h);
void*    reallocate(void* p, size_t o, size_t n, bool h);
void*    duplicate(const void* p, size_t n, bool h);
void*    deallocate(void* p, size_t n, bool h);
rl_sig_t collect_garbage(void);
void     push_gray(void* o);

#endif
