#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "runtime.h"
#include "labels.h"

// Stores type information and runtime methods
struct VTable {
  Type    code;
  hash64  hash;
  size64  obsize;
  TraceFn tracefn;
  FreeFn  freefn;
  HashFn  hashfn;
  EgalFn  egalfn;
  OrderFn orderfn;
};

/* External APIs */
void rl_init_vtables(State* vm);
void rl_toplevel_init_vtables(void);

#endif
