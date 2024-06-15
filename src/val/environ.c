#include "val/environ.h"
#include "val/text.h"
#include "val/type.h"

#include "util/text.h"
#include "util/hash.h"
#include "util/number.h"

/* Forward declarations */
bool egal_syms(Val x, Val y);
int  order_syms(Val x, Val y);

/* Internal APIs */

/* Internal Sym APIs */
Sym* new_sym(Str* n, Str* ns, word_t idno, bool lit) {
  Sym* o     = new_obj(&SymType);
  o->name    = n;
  o->nmspc   = ns;
  o->idno    = idno;
  o->literal = lit;

  hash_t h = n->hash;

  if ( ns != NULL )
    h = mix_hashes(h, ns->hash);

  if ( idno > 0 )
    h = mix_hashes(h, hash_word(idno));

  o->hash = h;

  return o;
}

bool egal_syms(Val x, Val y) {
  Sym* sx = as_sym(x), * sy = as_sym(y);

  return sx->name == sy->name && sx->nmspc == sy->nmspc && sx->idno == sy->idno;
}

int order_syms(Val x, Val y) {
  Sym* sx = as_sym(x), * sy = as_sym(y);

  int o = scmp(sx->name->chars, sy->name->chars);

  if ( o == 0 ) {
    Str* xns = get_ns(sx), * yns = get_ns(sy);

    o = scmp(xns->chars, yns->chars);
  }

  if ( o == 0 )
    o = cmp(sx->idno, sy->idno);

  return o;
}

/* Internal Ref APIs */
void init_ref_flags(Ref* r) {
  // initialize C flags from supplied metadata
  r->macro = meta_eq(r, ":macro", TRUE);
  r->mm    = meta_eq(r, ":multi", TRUE);
  r->final = meta_eq(r, ":final", TRUE);

  Val tag  = get_meta(r, ":tag");

  if ( tag != NOTHING && is_type(tag) ) // TODO: should validate, not ignore
    r->tag = as_type(tag);

  else {
    r->tag = &AnyType;
    set_meta(r, ":tag", tag(&AnyType));
  }
}

void init_ref_meta(Ref* r) {
  // initialize metadata from C flags (for Refs created in C code)

  if ( r->macro )
    set_meta(r, ":macro", TRUE);

  if ( r->mm )
    set_meta(r, ":multi", TRUE);

  if ( r->final )
    set_meta(r, ":final", TRUE);

  if ( r->tag == NULL )
    r->tag = &AnyType;

  set_meta(r, ":tag", tag(r->tag));
}

/* External APIs */
/* Symbol APIs */

Sym* c_mk_sym(char* n, char* ns, bool gs) {
  Str* sn, * sns;
  size_t nc, nsc;

  nc = scnt(n);
  sn = mk_str(n, nc);

  if ( ns ) {
    nsc = scnt(ns);
    sns = mk_str(ns, nsc);
  } else {
    sns = NULL;
  }

  return mk_sym(sn, sns, gs);
}

Sym* s_mk_sym(Str* n, Str* ns, bool gs) {
  static word_t gsc = 1;
  bool literal = n->chars[0] == ':';
  Sym* o;

  if ( gs )
    o = new_sym(n, ns, gsc++, literal);

  else
    o = new_sym(n, ns, 0, literal);

  return o;
}

bool s_sn_eq(Sym* s, const char* n) {
  return seq(s->name->chars, n);
}

bool v_sn_eq(Val x, const char* n) {
  assert(is_sym(x));

  return sn_eq(as_sym(x), n);
}

// qualify methods
Sym* cstr_qualify(Sym* s, char* ns);
Sym* str_qualify(Sym* s, Str* ns);
Sym* sym_qualify(Sym* s, Sym* ns);

/* Ref APIs */
Ref* mk_ref(Scope* scope, Env* environ);
