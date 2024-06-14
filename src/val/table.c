#include "val/table.h"
#include "val/type.h"
#include "val/text.h"
#include "val/environ.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/text.h"
#include "util/hash.h"

/* Internal forward declarations */


/* Globals */
/* HAMT parameters */
#define OMAP_LIMIT     16
#define LEVEL_SHIFT     6
#define MAX_SHIFT       8
#define LEVEL_SIZE     64
#define LEVEL_MASK     63

/* Table parameters */
#define MIN_TABLE_SIZE  8

/* Internal APIs */

/* External APIs */

/* Mutable tables */
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
#undef OMAP_LIMIT
#undef LEVEL_SHIFT
#undef MAX_SHIFT
#undef LEVEL_SIZE
#undef LEVEL_MASK
