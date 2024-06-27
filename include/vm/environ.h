#ifndef rl_vm_environ_h
#define rl_vm_environ_h

#include "val/table.h"
#include "val/array.h"

/* Global state object for managing namespaces and environments. */
struct Environ {
  Alist  stk;    // stack of pending maespaces
  Env*   gl;     // default global namespace
  Env*   ns;     // current namespace
  Env*   nss;    // 
};

#endif
