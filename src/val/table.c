#include <string.h>

#include "val/table.h"
#include "val/type.h"
#include "val/text.h"
#include "val/environ.h"
#include "val/list.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/number.h"
#include "util/text.h"
#include "util/hash.h"
#include "util/bits.h"

/* C types */
/* Internal forward declarations */
/* Internal Map & MNode APIs */
/* Constructors & allocation */
Map*   new_map(size_t n, Val *es, bool a, bool s, bool t);
MNode* new_mnode(size_t s, size_t m, Obj** cn, bool t);
void   resize_mn(MNode* n, size_t i);

/* Clone methods */
void   clone_map(void* x);
void   clone_mnode(void* x);

/* Unpack methods */
void   unpack_map(Map* m, Alist* b);
void   unpack_mnode(MNode* n, Alist* b);

/* transient methods */
Map*   transient_map(Map* m);
MNode* transient_mnode(MNode* n);

/* persistent methods */
Map*   persistent_map(Map* m);
MNode* persistent_mnode(MNode* n);

/* Find methods */
Pair* map_find(Map* m, Val k);
Pair* amap_find(Map* m, Val k);
Pair* mnode_find(MNode* n, Val k, hash_t h);

/* add_to methods */
bool   add_to_map(Map* m, Val k, Val v);
bool   add_to_amap(Map* m, Val k, Val v, hash_t h);
bool   add_to_mnode(MNode** n, Val k, Val v, hash_t h);

/* pop_from methods */
Map*   pop_from_map(Map* m, Val k, Pair** b);
Map*   pop_from_amap(Map* m, Val k, Pair** b);
MNode* pop_from_mnode(MNode* n, Val k, Pair** b, hash_t h);

/* Miscellaneous HAMT APIs */
Map*   amap_to_hmap(Map* m);
bool   is_cnode(MNode* n);
size_t mn_asize(MNode* n);
Pair*  mk_kv(Val k, Val v, hash_t h);
size_t get_aidx(hash_t h, size_t s);
size_t get_amask(size_t i);
bool   has_bidx(size_t i, size_t m);
size_t get_bidx(size_t i, size_t m);

/* Generics */
#define persistent(x)                           \
  generic((x),                                  \
          Map*:persistent_map,                  \
          MNode*:persistent_mnode)(x)

#define transient(x)                            \
  generic((x),                                  \
          Map*:transient_map,                   \
          MNode*:transient_mnode)(x)

#define unpack(x, a)                            \
  generic((x),                                  \
          Map*:unpack_map,                      \
          MNode*:unpack_mnode)(x, a)

/* Globals */
/* HAMT parameters */
#define AMAP_LIMIT     16
#define HAMT_LSHIFT     6
#define HAMT_MSHIFT    48
#define HAMT_CSHIFT    63
#define HAMT_SIZE      64
#define HAMT_MASK      63

/* Table parameters */
#define MIN_TABLE_SIZE  8

/* Internal APIs */
/* Internal Map APIs */
/* Constructors */
Map* new_map(size_t n, Val *es, bool a, bool s, bool t) {
  Map* o   = new_obj(&MapType);
  o->cnt   = 0;
  o->trans = true;
  o->amap  = a;
  o->root  = NULL;

  if ( n > 0 ) {
    preserve(1, tag(o));

    if ( a )
      o->kvs = allocate( AMAP_LIMIT * sizeof(Pair*), false);

    if ( s )
      for ( size_t i=0; i<n; i++ )
        add_to_map(o, es[i], TRUE);

    else {
      assert((n & 1) == 0); // should already be validated
      
      for ( size_t i=0; i<n; i += 2 )
        add_to_map(o, es[i], es[i+1]);
    }
  }

  if ( !t )
    persistent(o);

  return o;
}

MNode* new_mnode(size_t s, size_t m, Obj** cn, bool t) {
  MNode* o = new_obj(&MNodeType);
  size_t c = popc(m);
  o->shift = s;
  o->bmap  = m;
  o->cn    = allocate(c*sizeof(Obj*), false);
  o->trans = t;

  if ( cn )
    memcpy(o->cn, cn, c*sizeof(Obj*));

  return o;
}

void resize_mn(MNode* n, size_t i) {
  size_t om = n->bmap;
  size_t nm = om | (1ul << i);
  size_t os = popc(om);
  size_t ns = popc(nm);

  if ( os == 0 )
    n->cn = allocate(ns*sizeof(Obj*), false);

  else
    n->cn = reallocate(n->cn, os*sizeof(Obj*), ns*sizeof(Obj*), false);

  n->bmap = nm;
}

/* Clone methods */
void clone_map(void* x) {
  Map* m = x;

  if ( m->amap )
    m->kvs = duplicate(m->kvs, AMAP_LIMIT*sizeof(Pair*), false);
}

void clone_mnode(void* x) {
  MNode* n = x;
  n->cn    = duplicate(n->cn, mn_asize(n)*sizeof(Obj*), false);
}

/* Unpack methods */
void unpack_map(Map* m, Alist* b) {
  if ( m->cnt > 0 ) {
    if ( m->amap )
      write_alist(b, (void**)m->kvs, m->cnt);

    else
      unpack(m->root, b);
  }
}

void unpack_mnode(MNode* n, Alist* b) {
  size_t as = mn_asize(n);

  for ( size_t i=0; i < as; i++ ) {
    Obj* c = n->cn[i];

    if ( c->type == &PairType )
      alist_push(b, c);

    else
      unpack((MNode*)c, b);
  }
}

/* Transient methods */
Map* transient_map(Map* m) {
  if ( !m->trans ) {
    m = clone_obj(m);
    m->trans = true;
    m->hash = 0;      // invalidate hash if present
  }

  return m;
}

MNode* transient_mnode(MNode* n) {
  if ( n && !n->trans ) {
      n = clone_obj(n);
      n->trans = true;
  }

  return n;
}

/* persistent methods */
Map* persistent_map(Map* m) {
  if ( m->trans ) {
    m->trans = false;

    if ( !m->amap )
      persistent(m->root);
  }

  return m;
}

MNode* persistent_mnode(MNode* n) {
  if ( n && n->trans ) {
    n->trans = false;
    size_t c = mn_asize(n);

    for ( size_t i=0; i < c; i++ ) {
      Obj* c = n->cn[i];

      if ( c->type == &MNodeType )
        persistent((MNode*)n);
    }
  }

  return n;
}

/* Find methods */
Pair* map_find(Map* m, Val k) {
  Pair* o = NULL;

  if ( m->cnt > 0 ) {
    if ( m->amap )
      o = amap_find(m, k);

    else
      o = mnode_find(m->root, k, rl_hash(k, false));
  }

  return o;
}


Pair* amap_find(Map* m, Val k) {
  Pair* o = NULL;
  
  for ( size_t i=0; o == NULL && i < m->cnt; i++ ) {
    Pair* kv = m->kvs[i];

    if ( rl_egal(kv->car, k) )
      o = kv;
  }

  return o;
}

Pair* mnode_find(MNode* n, Val k, hash_t h) {
  Pair* o = NULL;

  for (;;) {
    size_t s = n->shift;
    size_t m = n->bmap;
    size_t i = get_aidx(h, s);

    if ( has_bidx(i, m) ) {
      size_t b = get_bidx(i, m);
      Obj* c = n->cn[b];

      if ( c->type == &PairType ) { // leaf node
        Pair* kv = (Pair*)c;

        if ( rl_egal(kv->car, k) )
          o = kv;

        break;
      } else if ( c->type == &ListType ) { // collision node
        o = list_find((List*)c, k);
        break;
      } else { // internal node
        n = (MNode*)c;
      }
    } else {
      break;
    }
  }

  return o;
}

Pair* list_find(List* kvs, Val k) {
  Pair* o = NULL;

  while ( o == NULL && kvs->cnt ) {
    Pair* kv = as_pair(kvs->head);

    if ( rl_egal(k, kv->car) )
      o = kv;

    else
      kvs = kvs->tail;
  }

  return o;
}

/* add_to methods */
bool add_to_map(Map* m, Val k, Val v) {
  bool r;
  hash_t h = rl_hash(k, false);

  if ( m->amap ) {
    if ( m->cnt == AMAP_LIMIT ) {
      amap_to_hmap(m);
      r = add_to_mnode(&m->root, k, v, h);
    } else {
      r = add_to_amap(m, k, v, h);
    }
  } else {
    r = add_to_mnode(&m->root, k, v, h);
  }

  if ( r )
    m->cnt++;

  return r;
}

bool add_to_amap(Map* m, Val k, Val v, hash_t h) {
  bool f = false;

  for ( size_t i = 0; !f && i < m->cnt; i++ ) {
    Pair* kv = m->kvs[i];

    if ( rl_egal(kv->car, k) ) {
      f = true;
      m->kvs[i] = mk_kv(k, v, h);
    }
  }

  if ( !f )
    m->kvs[m->cnt+1] = mk_kv(k, v, h);

  return !f;
}

bool add_to_mnode(MNode** n, Val k, Val v, hash_t h);

/* Miscellaneous HAMT APIs */
size_t mn_asize(MNode* n) {
  return popc(n->bmap);
}

bool is_cnode(MNode* n) {
  return n->shift == HAMT_CSHIFT;
}

Pair* mk_kv(Val k, Val v, hash_t h) {
  Pair* kv = mk_pair(k, v);
  kv->hash = h;
  kv->ownh = false;

  return kv;
}

size_t get_aidx(hash_t h, size_t s) {
  return (h >> s) & HAMT_MASK;
}

size_t get_imask(size_t i) {
  return 1ul << i;
}

bool has_bidx(size_t i, size_t m) {
  return get_imask(i) & m;
}

size_t get_bidx(size_t i, size_t m) {
  return popc(m & (get_imask(i) - 1));
}

/* Internal mutable table APIs */
static inline double get_loadf(LF l) {
  double o;

  switch ( l ) {
    case LF_125: o = 0.125; break;
    case LF_250: o = 0.250; break;
    case LF_375: o = 0.375; break;
    case LF_500: o = 0.500; break;
    case LF_625: o = 0.625; break;
    case LF_750: o = 0.750; break;
    case LF_875: o = 0.875; break;
  }

  return o;
}

static bool check_table_resize(LF l, size_t c, size_t mc) {
  return ((double)c) / get_loadf(l) >= (double)mc;
}

/* External APIs */
/* Map APIs */
Map*  mk_map(size_t n, Val* kvs) {
  return new_map(n, kvs, n <= AMAP_LIMIT, false, false);
}

Map*  mk_set(size_t n, Val* ks) {
  return new_map(n, ks, n <= AMAP_LIMIT, false, true);
}

bool map_get(Map* m, Val k, Val* r) {
  Pair* p = map_find(m, k);
  bool o = p != NULL;

  if ( o && r )
    *r = p->cdr;

  return o;
}

bool map_has(Map* m, Val k) {
  return map_get(m, k, NULL);
}

Map* map_set(Map* m, Val k, Val v) {
  if ( !m->trans ) {
    m = transient(m);
    preserve(1, tag(m));
    add_to_map(m, k, v);
    m = persistent(m);
  } else {
    add_to_map(m, k, v);
  }

  return m;
}

Map* map_put(Map* m, Val k, Val v) {
  if ( !map_has(m, k) )
    m = map_set(m, k, v);

  return m;
}

Map* map_pop(Map* m, Val k, Pair** kv);

Map* join_maps(Map* x, Map* y) {
  Map* o;
  
  if ( is_emap(x) )
    o = y;

  else if ( is_emap(y) )
    o = x;

  else {
    o = transient(x);
    preserve(1, tag(o));

    if ( is_amap(y) ) {
      Alist* a = y->amap;

      for ( size_t i=0; i<a->cnt; i++ ) {
        Pair* kv = a->data[i];
        add_to_map(o, kv->car, kv->cdr);
      }
    } else {
      Alist b;
      unpack(y, &b);

      for ( size_t i=0; i<b.cnt; i++ ) {
        Pair* kv = b.data[i];
        add_to_map(o, kv->car, kv->cdr);
      }

      free_alist(&b);
    }

    o = persistent(o);
  }

  return o;
}


/* Mutable table APIs */
#define MUTABLE_TABLE(T, E, K, V, t, nk, nv, ifn, hfn, cfn, rhfn)       \
  T* new_##t(LF l, flags_t f) {                                         \
    T* o = new_obj(&T##Type);                                           \
    init_##t(o, l, f);                                                  \
    return o;                                                           \
  }                                                                     \
                                                                        \
  void init_##t(T* t, LF l, flags_t f) {                                \
    t->lf = l;                                                          \
    t->entries = NULL;                                                  \
    t->cnt = 0;                                                         \
    t->maxc = 0;                                                        \
    t->nts = 0;                                                         \
    ifn(t, f);                                                          \
  }                                                                     \
                                                                        \
  void free_##t(void* x) {                                              \
    T* t = x;                                                           \
                                                                        \
    deallocate(t->entries, 0, false);                                   \
    init_##t(t, t->lf, 0);                                              \
  }                                                                     \
                                                                        \
  static inline bool is_##t##_ts(E* e) {                                \
    return e->key == nk && e->val != nv;                                \
  }                                                                     \
                                                                        \
  static void init_##t##_es(E* es, size_t maxc) {                       \
    for ( size_t i=0; i<maxc; i++ ) {                                   \
      es[i].key = nk;                                                   \
      es[i].val = nv;                                                   \
    }                                                                   \
  }                                                                     \
                                                                        \
  static E* t##_locate(E* es, K k, size_t maxc, hash_t h) {             \
    size_t m = maxc - 1;                                                \
    size_t i = h & m;                                                   \
    E* ts = NULL, * e;                                                  \
                                                                        \
    for (;;) {                                                          \
      e = &es[i];                                                       \
                                                                        \
      if ( e->key == nk ) {                                             \
        if ( e->val == nv ) { /* Empty entry */                         \
          e = ts ? : e;                                                 \
          break;                                                        \
        } else {                                                        \
          if ( ts == NULL )                                             \
            ts = e;                                                     \
        }                                                               \
      } else if ( cfn(k, e->key) ) {                                    \
        break;                                                          \
      }                                                                 \
      i = (i + 1) & m;                                                  \
    }                                                                   \
                                                                        \
    return e;                                                           \
  }                                                                     \
                                                                        \
  static void adjust_##t##_size(T* t, size_t nmc) {                     \
    E* nes = allocate(nmc*sizeof(E), false);                            \
    init_##t##_es(nes, nmc);                                            \
                                                                        \
    if ( t->entries != NULL ) {                                         \
      size_t nc = 0;                                                    \
                                                                        \
      for (size_t i=0; i<t->maxc && nc < t->cnt; i++) {                 \
        E* o = &t->entries[i];                                          \
                                                                        \
        if ( o->key == nk )                                             \
          continue;                                                     \
                                                                        \
        K k = o->key;                                                   \
        V v = o->val;                                                   \
        hash_t h = rhfn(o);                                             \
        E* n = t##_locate(nes, k, nmc, h);                              \
        n->key = k;                                                     \
        n->val = v;                                                     \
        nc++;                                                           \
      }                                                                 \
                                                                        \
      deallocate(t->entries, 0, false);                                 \
      t->cnt = nc;                                                      \
      t->nts = 0;                                                       \
    }                                                                   \
                                                                        \
    t->maxc = nmc;                                                      \
  }                                                                     \
                                                                        \
  E* t##_find(T* t, K k) {                                              \
    E* e = NULL;                                                        \
    if ( t->entries != NULL )                                           \
      e =  t##_locate(t->entries, k, t->maxc, hfn(k));                  \
    return e;                                                           \
  }                                                                     \
                                                                        \
  E* t##_intern(T* t, K k, rl_intern_fn_t f, void* s) {                 \
    if ( check_table_resize(t->lf, t->cnt+1, t->maxc) )                 \
      adjust_##t##_size(t, t->maxc ? t->maxc << 1 : MIN_TABLE_SIZE);    \
                                                                        \
    hash_t h = hfn(k);                                                  \
    E* e = t##_locate(t->entries, k, t->maxc, h);                       \
    bool fr = e->key == nk;                                             \
    bool ts = fr && e->val != nv;                                       \
                                                                        \
    if ( fr ) {                                                         \
      f(t, e, (void*)k, s, h);                                          \
                                                                        \
      if ( ts )                                                         \
        t->nts--;                                                       \
                                                                        \
      else                                                              \
        t->cnt++;                                                       \
    }                                                                   \
                                                                        \
    return e;                                                           \
  }                                                                     \
                                                                        \
  bool t##_get(T* t, K k, V* r) {                                       \
    E* e = t##_find(t, k);                                              \
    bool o;                                                             \
                                                                        \
    if ( e != NULL ) {                                                  \
      o = e->key != nk;                                                 \
      if ( o && r )                                                     \
        *r = e->val;                                                    \
    } else {                                                            \
      o = false;                                                        \
    }                                                                   \
                                                                        \
    return o;                                                           \
  }                                                                     \
                                                                        \
  bool t##_has(T* t, K k) {                                             \
    return t##_get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool t##_set(T* t, K k, V v, V* r) {                                  \
    if ( check_table_resize(t->lf, t->cnt+1, t->maxc) )                 \
      adjust_##t##_size(t, t->maxc ? t->maxc << 1 : MIN_TABLE_SIZE);    \
                                                                        \
    hash_t h = hfn(k);                                                  \
    E* e = t##_locate(t->entries, k, t->maxc, h);                       \
    bool fr = e->key == nk;                                             \
    bool ts = fr && e->val != nv;                                       \
                                                                        \
    if ( fr ) {                                                         \
      e->key = k;                                                       \
                                                                        \
      if ( ts )                                                         \
        t->nts--;                                                       \
                                                                        \
      else                                                              \
        t->cnt++;                                                       \
    }                                                                   \
                                                                        \
    if ( r )                                                            \
      *r = fr ? v : e->val;                                             \
                                                                        \
    e->val = v;                                                         \
    return fr;                                                          \
  }                                                                     \
                                                                        \
  bool t##_put(T* t, K k, V v) {                                        \
    if ( check_table_resize(t->lf, t->cnt+1, t->maxc) )                 \
      adjust_##t##_size(t, t->maxc ? t->maxc << 1 : MIN_TABLE_SIZE);    \
                                                                        \
    hash_t h = hfn(k);                                                  \
    E* e = t##_locate(t->entries, k, t->maxc, h);                       \
    bool fr = e->key == nk;                                             \
    bool ts = fr && e->val != nv;                                       \
                                                                        \
    if ( fr ) {                                                         \
      e->key = k;                                                       \
                                                                        \
      if ( ts )                                                         \
        t->nts--;                                                       \
                                                                        \
      else                                                              \
        t->cnt++;                                                       \
                                                                        \
      e->val = v;                                                       \
    }                                                                   \
                                                                        \
    return fr;                                                          \
  }                                                                     \
                                                                        \
  bool t##_del(T* t, K k, V* r) {                                       \
    bool o;                                                             \
                                                                        \
    if ( t->entries != NULL ) {                                         \
      E* e = t##_find(t, k);                                            \
      o = e->key != nk;                                                 \
                                                                        \
      if ( o ) {                                                        \
        e->key = nk;                                                    \
        t->nts++;                                                       \
                                                                        \
        if ( r )                                                        \
          *r = e->val;                                                  \
      }                                                                 \
    } else {                                                            \
      o = false;                                                        \
                                                                        \
      if ( r )                                                          \
        *r = nv;                                                        \
    }                                                                   \
    return o;                                                           \
  }                                                                     \
                                                                        \
  T* join_##t##s(T* x, T* y) {                                          \
    size_t na = 0;                                                      \
    for ( size_t i=0; i<y->maxc && na<y->cnt; i++ ) {                   \
      E* e = &y->entries[i];                                            \
                                                                        \
      if ( e->key == nk )                                               \
        continue;                                                       \
                                                                        \
      t##_set(x, e->key, e->val, NULL);                                 \
      na++;                                                             \
    }                                                                   \
                                                                        \
    return x;                                                           \
  }                                                                     \

  
  
static inline void init_mmap_flags(void* t, flags_t f) {
  (void)t;
  (void)f;
}

static inline hash_t hash_mm_key(Val k) {
  return rl_hash(k, false);
}

static inline hash_t rehash_mm_entry(MMEntry* e) {
  return rl_hash(e->key, false);
}

MUTABLE_TABLE(MMap,
              MMEntry,
              Val, Val,
              mmap,
              NOTHING, NOTHING,
              init_mmap_flags, hash_mm_key, rl_egal, rehash_mm_entry);

static inline void init_sc_flags(void* t, flags_t f) {
  (void)t;
  (void)f;
}

static inline hash_t rehash_sc_entry(SCEntry* e) {
  return e->val->hash;
}

MUTABLE_TABLE(SCache,
              SCEntry,
              char*, Str*,
              scache,
              NULL, NULL,
              init_sc_flags, hash_chars, seq, rehash_sc_entry);

static inline void init_em_flags(void* t, flags_t f) {
  EMap* m = t;

  m->scope = f;
}

static inline hash_t hash_em_key(Sym* k) {
  return k->hash;
}

static inline bool cmp_em_keys(Sym* x, Sym* y) {
  return x->name == y->name && get_ns(x) == get_ns(y) && x->idno == y->idno;
}

static inline hash_t rehash_em_entry(EMEntry* e) {
  return e->key->hash;
}

MUTABLE_TABLE(EMap,
              EMEntry,
              Sym*, Ref*,
              emap,
              NULL, NULL,
              init_em_flags, hash_em_key, cmp_em_keys, rehash_em_entry);

#undef MUTABLE_TABLE
#undef AMAP_LIMIT
#undef HAMT_SHIFT
#undef HAMT_MSHIFT
#undef HAMT_SIZE
#undef HAMT_MASK
