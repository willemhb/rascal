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
rl_status_t hstate_mark(HState* h);
rl_status_t hstate_push(HState* h, Object* o);
rl_status_t hstate_pushn(HState* h, size_t n, ...);
rl_status_t hstate_pop(HState* h, Object** o);
rl_status_t hstate_popn(HState* h, Object** o, bool t, size_t n);

// preserving values
void unpreserve(HFrame* frame);

#define preserve(n, vals...)                                    \
  Value __heap_frame_vals__[(n)] = { vals };                    \
  HFrame __heap_frame__ cleanup(unpreserve) =                   \
    { .next=Heap.fp, .count=(n), .values=__heap_frame_vals__ }; \
  Heap.fp=&__heap_frame__

// memory management
rl_status_t allocate(void** b, size_t n, bool h);
rl_status_t reallocate(void** b, size_t p, size_t n, bool h);
rl_status_t duplicate(const void* p, void** b, size_t n, bool h);
rl_status_t deallocate(void* p, size_t n, bool h);
rl_status_t collect_garbage(void);
rl_status_t push_gray(void* o);

#endif
