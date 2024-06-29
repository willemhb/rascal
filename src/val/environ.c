#include "val/environ.h"
#include "val/text.h"
#include "val/type.h"
#include "val/table.h"
#include "val/array.h"

#include "vm/heap.h"
#include "vm/environ.h"

#include "util/text.h"
#include "util/hash.h"
#include "util/number.h"

/* Forward declarations */
bool egal_syms(Val x, Val y);
int  order_syms(Val x, Val y);

/* Internal APIs */
typedef struct {
  Env* e;
  Ref* c;
} IRefState;

void intern_ref(void* t, void* e, void* k, void* s, hash_t h) {
  EMap*     m   = t;
  EMEntry* me   = e;
  IRefState* is = s;
  me->key       = k;
  me->val       = mk_ref(m->scope, is->e, is->c, k);
  me->val->hash = h;
  me->val->ownh = false;
}

Ref* capture_ref(Env* e, Sym* n, Ref* r) {
  IRefState is = { .e=e, .c=r };
  EMEntry*  em = emap_intern(e->captured, n, intern_ref, &is);

  return em->val;
}

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
  r->macro   = meta_eq(r, ":macro", TRUE);
  r->mm      = meta_eq(r, ":multi", TRUE);
  r->final   = meta_eq(r, ":final", TRUE);   // TODO: should be default
  r->private = meta_eq(r, ":private", TRUE);
  Val tag    = get_meta(r, ":tag");

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

  if ( r->private )
    set_meta(r, ":private", TRUE);

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

// qualify/unqualify methods
Sym* unqualify(Sym* s) {
  return mk_sym(s->name, NULL, false);
}

Sym* cstr_qualify(Sym* s, char* ns) {
  Str* nss = mk_str(ns, 0);

  preserve(1, tag(nss));

  return str_qualify(s, nss);
}

Sym* str_qualify(Sym* s, Str* ns) {
  return mk_sym(ns, s->name, false);
}

Sym* sym_qualify(Sym* s, Sym* ns) {
  return str_qualify(s, ns->name);
}

/* Env APIs */
size_t lref_cnt(Env* e) {
  return e->locals->cnt;
}

size_t uref_cnt(Env* e) {
  if ( e->captured == NULL )
    return 0;

  return e->captured->cnt;
}

Env* mk_env(Scope s, Env* p, Sym* n, int i, bool b) {
  Env* o      = new_obj(&EnvType);
  o->scope    = s;
  o->bound    = b;
  o->index    = i;
  o->name     = n;
  o->parent   = p;

  preserve(1, tag(o));

  if ( s == LOCAL_SCOPE ) {
    assert(p != NULL);

    o->ns         = p->ns;

    if ( b ) {
      o->locals   = p->locals;
      o->captured = p->captured;
      o->refs     = p->refs;
      o->upvals   = allocate(uref_cnt(p) * sizeof(UpVal*), false);

    } else {
      o->locals   = new_emap(0, LOCAL_SCOPE);
      o->captured = new_emap(0, UPVALUE_SCOPE);
      o->refs     = new_alist(NULL, 0, true, RESIZE_PYTHON);
      o->upvals   = NULL;
    }
  }

  else if ( s == NAMESPACE_SCOPE ) {
    assert(i > -1);

    o->locals   = new_emap(0, NAMESPACE_SCOPE);
    o->captured = NULL;
    o->refs     = new_alist(NULL, 0, true, RESIZE_PYTHON);
    o->upvals   = NULL;
  }

  return o;
}

Env* bind_env(Env* t) {
  return mk_env(LOCAL_SCOPE, t, t->name, -1, true);
}

bool env_get(Env* e, Sym* n, int c, Ref** r) {
  bool o = false;

  if ( e == NULL )
    e = Environ.gns;

  Scope s = e->scope;

  if ( s == LOCAL_SCOPE ) {
    o = emap_get(e->locals, n, r); // check innermost scope first

    if ( o == false ) { // if not found, check if upvalue has already been captured
      o = emap_get(e->captured, n, r);
      
      if ( o == false ) { // check parent scope
        if ( c > -1 ) {
          o = env_get(e->parent, n, 1, r);
          
          if ( o == true && (*r)->scope == UPVALUE_SCOPE ) // capture upvalue from parent scope
            *r = capture_ref(e, n, *r);
        }
        
        else
          o = env_get(e->parent, n, -1, r);
      }
      
    } else if ( c == 1 ) // capture local reference
      *r = capture_ref(e, n, *r);
    
  } else {
    o = emap_get(e->locals, n, r);
    
    if ( o == false ) {
      if ( e->toplevel ) {
        if ( n->nmspc != NULL ) { // try qualified lookup
          Str* ns = n->nmspc;

          if ( nsmap_get(Environ.nss, ns, &e) )
            o = emap_get(e->locals, unqualify(n), r);
        }
      } else {
        o = env_get(e->parent, n, c, r);
      }
    }
  }

  return o;
}

bool env_put(Env* e, Sym* n, Ref** r) {
  if ( e == NULL )
    e = Environ.gns;

  IRefState s = { .e=e, .c=*r };
  int lr      = lref_cnt(e);
  EMEntry* em = emap_intern(e->locals, n, intern_ref, &s);
  *r          = em->val;

  return lr == em->val->offx;
}

/* Ref APIs */
Ref* mk_ref(Scope s, Env* e, Ref* c, Sym* n) {
  Ref* r      = new_obj(&RefType);
  r->scope    = s;
  r->meta     = n->meta;
  r->captures = c;
  r->environ  = e;
  r->name     = n;

  if ( s != UPVALUE_SCOPE ) {
    r->offx = lref_cnt(e);
    r->offy = e->index;
    r->val  = NOTHING;

    alist_push(e->refs, r);
    
  } else {
    assert(c != NULL);

    r->offx = uref_cnt(e);
    r->offy = c->offx;
    r->val  = NOTHING;
  }

  // initialize flags (updating metadata might trigger GC)
  preserve(1, tag(r));
  init_ref_flags(r);

  return r;
}

bool is_lupv(Ref* r) {
  return r->captures != NULL && r->environ == r->captures->environ;
}

/* UpVal APIs */
UpVal* mk_upv(UpVal* n_u, Val* l) {
  UpVal* o  = new_obj(&UpValType);
  o->closed = false;
  o->n_u    = n_u;
  o->l      = l;

  return o;
}

Val* dr_upv(UpVal* u) {
  if ( u->closed )
    return &u->v;

  return u->l;
}

UpVal* open_upv(UpVal** o, Val* l) {
  while (*o && (*o)->l > l )
    o = &(*o)->n_u;

  if ( *o == NULL )
    *o = mk_upv(NULL, l);

  else if ((*o)->l < l )
    *o = mk_upv((*o)->n_u, l);

  return *o;
}

void close_upv(UpVal** o, UpVal* u) {
  if ( !u->closed ) {
    if ( o ) {
      while ( *o && (*o)->l > u->l )
        o = &(*o)->n_u;

      (*o)->n_u = u->n_u;
    }

    Val v     = *u->l;
    u->n_u    = NULL;
    u->v      = v;
    u->closed = true;
  }
}

void close_upvs(UpVal** o, Val* l) {
  while (*o && (*o)->l >= l ) {
    UpVal* u = *o;
    *o       = u->n_u;

    close_upv(NULL, u);
  }
}
