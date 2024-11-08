#include "val/environ.h"
#include "val/text.h"

#include "vm/state.h"
#include "vm/heap.h"
#include "vm/type.h"

#include "util/text.h"
#include "util/hash.h"

/* Internal APIs */
void trace_sym(State* vm, void* x) {
  Sym* sym = x;

  mark(vm, sym->name);
  mark(vm, sym->ns);
}

bool egal_syms(Val x, Val y) {
  
}

int  order_syms(Val x, Val y);

void init_sym(State* vm, Sym* sym, Str* n, Str* ns, bool lit, bool gs) {
  // Gensym counter
  static size64 gscnt = 0;
  
  sym->literal = lit;
  sym->idno    = gs ? ++gscnt : 0;
  sym->name    = n;
  sym->ns      = ns;

  hash64 shash = n->chash;

  if ( ns )
    shash = mix_hashes(ns->chash, shash);

  if ( gs )
    shash = mix_hashes(hash_word(sym->idno), shash);

  shash = mix_hashes(vm->vts[T_SYM].hash, shash);

  sym->hash = shash;
}

/* External APIs */
Sym* new_sym(Str* n, Str* ns, bool lit, bool gs) {
  preserve(&Vm, 2, tag(n), tag(ns));
  Sym* sym = new_obj(&Vm, T_SYM, MF_PERSISTENT);

  init_sym(&Vm, sym, n, ns, lit, gs);

  return sym;
}

Sym* get_sym(char* n, char* ns, bool gs) {
  size64 nc, nsc;
  bool lit;
  Str* no, * nso;
  Sym* sym;

  nc = scnt(n);
  no = get_str(n, nc);

  if ( ns != NULL ) {
    preserve(&Vm, 1, tag(no));

    lit = ns[0] == ':';
    nsc = scnt(ns);
    nso = get_str(ns, nsc);
  } else {
    lit = n[0] == ':';
    nso = NULL;
  }

  sym = new_sym(no, nso, lit, gs);

  return sym;
}
