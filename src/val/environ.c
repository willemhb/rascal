#include "val/environ.h"
#include "val/text.h"

#include "vm/state.h"
#include "vm/heap.h"
#include "vm/type.h"

#include "util/text.h"
#include "util/number.h"
#include "util/hash.h"

/* Globals */
extern Str* DefaultNs;

/* Internal APIs */
static Str* get_ns(Sym* s) {
  return s->ns ? : DefaultNs;
}

void init_sym(State* vm, Sym* sym, Str* n, Str* ns, bool lit, bool gs) {
  // Gensym counter
  static size64 gscnt = 0;
  
  sym->lit  = lit;
  sym->id = gs ? ++gscnt : 0;
  sym->n    = n;
  sym->ns   = ns;

  hash64 shash = n->chash;

  if ( ns )
    shash = mix_hashes(ns->chash, shash);

  if ( gs )
    shash = mix_hashes(hash_word(sym->id), shash);

  shash = mix_hashes(vm->vts[T_SYM].hash, shash);

  sym->hash = shash;
}

/* Interfaces */
// lifetime
void trace_sym(State* vm, void* x) {
  Sym* sym = x;

  mark(vm, sym->n);
  mark(vm, sym->ns);
}

// comparison
bool egal_syms(Val x, Val  y) {
  Sym* sx  = as_sym(x), * sy = as_sym(y);

  return sx->ns == sy->ns && sx->n == sy->n && sx->id == sy->id;
}

int order_syms(Val x, Val y) {
  Sym* sx = as_sym(x), * sy = as_sym(y);
  int o = 0;
  Str* nsx = get_ns(sx), * nsy = get_ns(sy);

  // order on namespace, name, then idno
  o = order_str_obs(nsx, nsy);
  o = o ? : order_str_obs(sx->n, sy->n);
  o = o ? : cmp(sx->id, sy->id);

  return o;
}

// print
size64 pr_sym(State* vm, Port* p, Val x) {
  (void)vm;

  Sym* s = as_sym(x);

  size64 r;

  if ( s->ns )
    r = rl_printf(p, "%s/%s", s->ns, s->n );

  else
    r = rl_printf(p, "%s", s->n);

  if ( s->id )
    r += rl_printf(p, "#%zu", s->id);

  return r;
}

/* External APIs */
Sym* new_sym(Str* n, Str* ns, bool lit, bool gs) {
  preserve(&Vm, 2, tag(n), tag(ns));
  Sym* sym = new_obj(&Vm, T_SYM, MF_SEALED);

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
