#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "runtime.h"
#include "labels.h"

// Stores type information and runtime methods
struct VTable {
  // metadata
  Type    code;
  char*   name;    // for error messages
  hash64  hash;

  // layout information
  size64  obsize;

  // interface information
  bool    is_seq;
  bool    is_iseq;

  // lifetime methods
  TraceFn tracefn;
  FreeFn  freefn;
  CloneFn clonefn;
  SealFn  sealfn;

  // print interface
  PrFn    prfn;

  // comparison interface
  HashFn  hashfn;
  EgalFn  egalfn;
  OrderFn orderfn;

  // sequence interfaces
  // for values that implement directly (lists)
  EmptyFn  emptyfn;
  FirstFn  firstfn;
  RestFn   restfn;

  // for values that use an interface object (Vecs, Maps, Strs)
  SInitFn  sinitfn;
  SFirstFn sfirstfn;
  SRestFn  srestfn;
};

/* External APIs */
void rl_init_vtable(State* vm, VTable* vt);
void rl_init_types(State* vm);
void rl_toplevel_init_types(void);

#endif
