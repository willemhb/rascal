#ifndef rl_vm_environ_h
#define rl_vm_environ_h

#include "val/table.h"
#include "val/array.h"

/* Global state object for managing namespaces and environments. */
struct EState {
  Env*   gns;    // default global namespace
  NSMap* nss;    // cache of loaded namespaces (cached by both name and file path)
};

/* External APIs */
#define ref_gns(e, x)                                        \
  generic((x),                                               \
          Sym*:nref_gns,                                     \
          char*:sref_gns,                                    \
          int:iref_gns)(e, x)

#define ref_cns(e, x)                           \
  generic((x),                                  \
          Sym*:nref_cns,                        \
          char*:sref_cns,                       \
          int:iref_cns)(e, x)

#define ref_qns(e, x, ...)                                  \
  generic((x),                                              \
          Sym*:nref_qns,                                    \
          char*:sref_qns,                                   \
          int:iref_qns)(e, x, __VA_OPT__(,) __VA_ARGS__)

rl_err_t def_ns(Sym* n, Str* p, Env** r);

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
