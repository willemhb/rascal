#ifndef rl_vm_heap_h
#define rl_vm_heap_h

#include "vm/state.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */
/* C types */
/* Stores references to values that only exist on the C stack in case a garbage collection cycle occurs before an object is visible in Rascal. */
struct HFrame {
  HFrame* next;
  State*  vm;
  size64  cnt;
  Val*    saved;
};

/* Globals */
/* HState & Heap APIs */
// preserving values
void unpreserve(HFrame* f);

#define preserve(s, n, vals...)                                         \
  Val __heap_frame_vals__[(n)] = { vals };                              \
  HFrame __heap_frame__ cleanup(unpreserve) =                           \
    { .vm  =(s),                                                        \
      .next=(s)->hfs,                                                   \
      .cnt =(n),                                                        \
      .saved=__heap_frame_vals__ };                                     \
  (s)->hfs=&__heap_frame__

#define add_to_preserved(v, n)                  \
  __heap_frame_vals__[(n)] = (v)

// memory management
void* rl_alloc(State* vm, size64 n);
void* rl_realloc(State* vm, void* s, size64 o, size64 n);
void* rl_dup(State* vm, void* s, size64 n);
void  rl_dealloc(State* vm, void* s, size64 n);
void  rl_gc(State* vm, size64 n);
void  add_to_heap(State* vm, void* o);
void  push_gray(State* vm, void* o);
void* pop_gray(State* vm);

// utilities
void mark_vals(State* vm, size64 n, Val* vs);
void mark_objs(State* vm, size64 n, void* os);

#endif
