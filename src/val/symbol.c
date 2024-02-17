#include "util/unicode.h"
#include "util/hashing.h"

#include "vm/memory.h"
#include "vm/context.h"

#include "val/symbol.h"
#include "val/text.h"
#include "val/type.h"

/* Globals */
void trace_sym(void* obj);
hash_t hash_sym(Value x);
bool egal_syms(Value x, Value y);
int  order_syms(Value x, Value y);

INIT_OBJECT_TYPE(Symbol,
                 .tracefn=trace_sym,
                 .hashfn =hash_sym,
                 .egalfn =egal_syms,
                 .ordfn  =order_syms);

/* Internal APIs */
void trace_sym(void* obj) {
  Symbol* sym = obj;

  mark(sym->name);
  mark(sym->ns);
}

static hash_t compute_sym_hash(String* name, String* ns, idno_t id) {
  hash_t h = name->obj.hash;

  if (ns != NULL)
    h = mix_hashes(h, ns->obj.hash);

  if (id != 0)
    h = mix_hashes(h, hash_word(id));

  return h;
}

hash_t hash_sym(Value x) {
  Symbol* s = as_sym(x);

  return compute_sym_hash(s->name, s->ns, s->idno);
}

bool egal_syms(Value x, Value y) {
  // compare on namespace part, then name part, then idno part
  Symbol* sx = as_sym(x), * sy = as_sym(y);
  bool out = sx->ns == sy->ns && sx->name == sy->name && sx->idno == sy->idno;

  return out;
}

static int order_ns_parts(Symbol* sx, Symbol* sy) {
  String* nsx = sx->ns, * nsy = sy->ns;
  
  if (nsx == NULL)
    return nsy == NULL ? 0 : -1;

  else if (nsy == NULL)
    return 1;

  else
    return strcmp(nsx->chars, nsy->chars);
}

static int order_name_parts(Symbol* sx, Symbol* sy) {
  String* sxn = sx->name, * syn = sy->name;
  
  return strcmp(sxn->chars, syn->chars);
}

static int order_id_parts(Symbol* sx, Symbol* sy) {
  idno_t idx = sx->idno, idy = sy->idno;

  return 0 - (idx < idy) + (idx > idy);
}

int order_syms(Value x, Value y) {
  Symbol* sx = as_sym(x), * sy = as_sym(y);
  int o      = order_ns_parts(sx, sy);
  o          = o ? : order_name_parts(sx, sy);
  o          = o ? : order_id_parts(sx, sy);

  return o;
}

/* External APIs */
Symbol* mk_sym(String* name, String* ns, bool gs) {
  Symbol* out;
  hash_t sh;

  out          = new_obj(&SymbolType, 0, 0);
  out->name    = name;
  out->ns      = ns;
  out->idno    = gs ? ++Context.gensym_counter : 0;
  sh           = compute_sym_hash(name, ns, out->idno);

  set_obj_hash(out, sh);

  return out;
}
