#ifndef rl_vm_heap_h
#define rl_vm_heap_h

#include "vm/state.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */

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
      .vals=__heap_frame_vals__ };                                      \
  (s)->hfs=&__heap_frame__

// memory management
void* rl_alloc(State* vm, size64 n);
void* rl_realloc(State* vm, void* s, size64 o, size64 n);
void* rl_dup(State* vm, void* s, size64 n);
void  rl_dealloc(State* vm, void* s, size64 n);
void  rl_gc(State* vm, size64 n);
void  add_to_heap(State* vm, void* o);
void  push_gray(State* vm, void* o);
void* pop_gray(State* vm);

#endif
