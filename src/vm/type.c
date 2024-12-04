#include "val/environ.h"
#include "val/list.h"
#include "val/text.h"
#include "val/array.h"
#include "val/table.h"

#include "vm/state.h"
#include "vm/type.h"

#include "util/hash.h"

/* Forward declarations */
/* Globals */
VTable *Vts[N_TYPES] = {};

/* External APIs */
void rl_init_vtable(State* vm, VTable* vt) {
  Type t     = vt->code;
  vt->hash   = hash_word(t);
  vm->vts[t] = vt; 
}

void rl_init_types(State* vm) {
  // nothing else for it
  rl_init_vtable(vm, &NulVt);
  rl_init_vtable(vm, &BoolVt);
  rl_init_vtable(vm, &GlyphVt);
  rl_init_vtable(vm, &NumVt);
  rl_init_vtable(vm, &PtrVt);
  rl_init_vtable(vm, &PrimFnVt);
  rl_init_vtable(vm, &UserFnVt);
  rl_init_vtable(vm, &SymVt);
  rl_init_vtable(vm, &PortVt);
  rl_init_vtable(vm, &StrVt);
  rl_init_vtable(vm, &ListVt);
  rl_init_vtable(vm, &VecVt);
  rl_init_vtable(vm, &MapVt);
  rl_init_vtable(vm, &MapVt);
  rl_init_vtable(vm, &PairVt);
  rl_init_vtable(vm, &BufferVt);
  rl_init_vtable(vm, &AlistVt);
  rl_init_vtable(vm, &TableVt);
  rl_init_vtable(vm, &VNodeVt);
  rl_init_vtable(vm, &MNodeVt);
  rl_init_vtable(vm, &SeqVt);
  rl_init_vtable(vm, &RtVt);
  rl_init_vtable(vm, &NsVt);
  rl_init_vtable(vm, &EnvVt);
  rl_init_vtable(vm, &RefVt);
  rl_init_vtable(vm, &UpvVt);
}

void rl_toplevel_init_types(void) {
  rl_init_types(&Vm);
}
