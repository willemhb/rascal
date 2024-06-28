#include "vm/environ.h"

#include "val/environ.h"

Val nref_gns(EState* e, Sym* n);
Val sref_gns(EState* e, char* s);

Val iref_gns(EState* e, size_t o) {
  return e->gns->values->data[o];
}

Val nref_cns(EState* e, Sym* n);
Val sref_cns(EState* e, char* s);

Val iref_cns(EState* e, size_t o) {
  return e->cns->values->data[o];
}
