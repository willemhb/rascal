#ifndef rl_vm_environ_h
#define rl_vm_environ_h

#include "val/table.h"
#include "val/array.h"

/* Global state object for managing namespaces and environments. */
struct EState {
  Alist  stk;    // stack of pending maespaces
  Env*   gns;    // default global namespace
  Env*   cns;    // current namespace
  Env*   qns;    // qualified namespaces
};

/* External APIs */
#define ref_gns(e, x)                                        \
  generic((x),                                               \
          Sym*:nref_gns,                                     \
          char*:sref_gns,                                    \
          size_t:iref_gns)(e, x)

#define ref_cns(e, x)                           \
  generic((x),                                  \
          Sym*:nref_cns,                        \
          char*:sref_cns,                       \
          size_t:iref_cns)(e, x)

#define ref_qns(e, x, ...)                                  \
  generic((x),                                              \
          Sym*:nref_qns,                                    \
          char*:sref_qns,                                   \
          size_t:iref_qns)(e, x, __VA_OPT__(,) __VA_ARGS__)

Val nref_gns(EState* e, Sym* n);
Val sref_gns(EState* e, char* s);
Val iref_gns(EState* e, size_t o);

Val nref_cns(EState* e, Sym* n);
Val sref_cns(EState* e, char* s);
Val iref_cns(EState* e, size_t o);

Val nref_qns(EState* e, Sym* n);
Val sref_qns(EState* e, char* x, char* y);
Val iref_qns(EState* e, size_t i, size_t j);

#endif
