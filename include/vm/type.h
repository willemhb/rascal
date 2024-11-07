#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "runtime.h"
#include "labels.h"

// Stores type information and runtime methods
struct VTable {
  Type    code;
  hash64  thash;
  size64  obsize;
  TraceFn trace;
  FreeFn  free;
  HashFn  hash;
  EgalFn  egal;
  OrderFn order;
};

/* External APIs */
void rl_init_vtables(State* vm);

#endif
