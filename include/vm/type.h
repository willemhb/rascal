#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "runtime.h"
#include "labels.h"

// Stores type information and runtime methods
struct VTable {
  Type    code;
  char*   name;    // for error messages
  hash64  hash;
  size64  obsize;
  TraceFn tracefn;
  FreeFn  freefn;
  SealFn  sealfn;
  HashFn  hashfn;
  EgalFn  egalfn;
  OrderFn orderfn;
};

/* External APIs */
void rl_init_types(State* vm);
void rl_toplevel_init_types(void);

#endif
