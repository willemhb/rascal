#include <string.h>

#include "collection.h"
#include "runtime.h"
#include "util.h"
#include "data.h"

// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

// Binary API
// object for storing binary data
// alist implementation macro
#define ALIST_IMPL(A, X, a)                                         \
  void init_##a(A* a) {                                             \
    a->vals      = NULL;                                            \
    a->count     = 0;                                               \
    a->max_count = 0;                                               \
  }                                                                 \
                                                                    \
  void free_##a(A* a) {                                             \
    release(a->vals, 0);                                            \
    init_##a(a);                                                    \
  }                                                                 \
                                                                    \
  void grow_##a(A* a) {                                             \
    if ( a->max_count == MAX_ARITY )                                \
      runtime_error("maximum "#a" size exceeded");                  \
    int new_maxc  = a->max_count ? a->max_count << 1 : MIN_CAP;   \
    X*  new_spc  = reallocate(false,                              \
                              new_maxc * sizeof(X),               \
                              a->max_count * sizeof(X),           \
                              a->vals);                           \
                                                                  \
    a->vals = new_spc;                                            \
    a->max_count = new_maxc;                                      \
  }                                                               \
                                                                  \
  void shrink_##a(A* a) {                                         \
    assert(a->max_count > MIN_CAP);                               \
                                                                  \
    size_t new_maxc = a->max_count >> 1;                          \
    X*  new_spc     = reallocate(false,                           \
                                 new_maxc*sizeof(X),              \
                                 a->max_count*sizeof(X),          \
                                 a->vals);                        \
                                                                  \
    a->vals      = new_spc;                                       \
    a->max_count = new_maxc;                                      \
  }                                                               \
                                                                  \
  void resize_##a(A* a, int n) {                                  \
    if ( n > MAX_ARITY )                                          \
      runtime_error("maximum"#a"size exceeded");                  \
                                                                        \
    int new_maxc = cpow2(n);                                            \
                                                                        \
    if ( new_maxc < MIN_CAP )                                           \
      new_maxc = MIN_CAP;                                               \
                                                                        \
    X* new_spc = reallocate(false,                                      \
                            new_maxc*sizeof(X),                         \
                            a->max_count*sizeof(X),                     \
                            a->vals);                                   \
    a->vals         = new_spc;                                          \
    a->max_count    = new_maxc;                                         \
  }                                                                     \
                                                                        \
                                                                        \
  void a##_push(A* a, X x) {                                            \
    if ( a->count == a->max_count )                                     \
      grow_##a(a);                                                      \
                                                                        \
    a->vals[a->count++] = x;                                            \
  }                                                                     \
                                                                        \
  X a##_pop(A* a) {                                                     \
    X out = a->vals[--a->count];                                        \
                                                                        \
    if ( a->count == 0 )                                                \
      free_##a(a);                                                      \
                                                                        \
    else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) ) \
      shrink_##a(a);                                                    \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
                                                                        \
  void a##_write(A* a, X* xs, int n) {                                  \
    if ( a->count + n > a->max_count )                                  \
      resize_##a(a, a->count+n);                                        \
                                                                        \
    memcpy(a->vals+a->count, xs, n*sizeof(X));                          \
    a->count += n;                                                      \
  }

ALIST_IMPL(Exprs, Expr, exprs);
ALIST_IMPL(Objs, void*, objs);
ALIST_IMPL(Bin16, ushort_t, bin16);

// Table implementation macro
#define check_grow(t) ((t)->count >= ((t)->max_count * LOADF))

#define TABLE_IMPL(T, K, V, t, NK, NV, hashf, rehashf, cmpf)            \
  void   init_##t(T* t) {                                               \
    t->kvs       = NULL;                                                \
    t->count     = 0;                                                   \
    t->max_count = 0;                                                   \
  }                                                                     \
                                                                        \
  void free_##t(T* t) {                                                 \
    release(t->kvs, 0);                                                 \
    init_##t(t);                                                        \
  }                                                                     \
                                                                        \
  void init_##t##_kvs(T##KV* kvs, int max_count) {                      \
    for ( int i=0; i < max_count; i++ ) {                               \
      kvs[i].key = NK;                                                  \
      kvs[i].val = NV;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  T##KV* t##_find(T* t, K k, hash_t h) {                                \
    assert(t->kvs != NULL);                                             \
    T##KV* kvs = t->kvs, * kv, * ts = NULL;                             \
    int msk = t->max_count-1;                                           \
    int idx = h & msk;                                                  \
                                                                        \
    for (;;) {                                                          \
      kv = &kvs[idx];                                                   \
      if ( kv->key == NK ) {                                            \
        if ( kv->val != NV ) /* tombstone */                            \
          ts = ts ? : kv;                                               \
        else                                                            \
          break;                                                        \
      } else if ( cmpf(kv->key, k) )                                    \
        break;                                                          \
      idx = (idx + 1) & msk;                                            \
      }                                                                 \
                                                                        \
    return ts ? : kv;                                                   \
    }                                                                   \
                                                                        \
  int rehash_##t( T##KV* old, int omc, T##KV* new, int nmc ) {          \
    int cnt = 0;                                                        \
                                                                        \
    for ( int i=0; i < omc; i++ ) {                                     \
      T##KV* kv = &old[i];                                              \
      if ( kv->key == NK )                                              \
        continue;                                                       \
                                                                        \
      cnt++;                                                            \
                                                                        \
      hash_t hash = rehashf(kv);                                        \
      int msk  = nmc - 1;                                               \
      int idx  = hash & msk;                                            \
      while ( new[idx].key != NK )                                      \
        idx = (idx+1) & msk;                                            \
                                                                        \
      new[idx] = *kv;                                                   \
    }                                                                   \
    return cnt;                                                         \
  }                                                                     \
                                                                        \
    void grow_##t(T* t) {                                               \
      int nmc    = t->max_count < MIN_CAP ? MIN_CAP : t->max_count << 1; \
      T##KV*    nkv = allocate(false, nmc*sizeof(T##KV));               \
                                                                        \
      init_##t##_kvs(nkv, nmc);                                         \
                                                                        \
    if ( t->kvs != NULL ) {                                             \
      int omc = t->max_count;                                           \
      T##KV*    okv = t->kvs;                                           \
      int nc  = rehash_##t(okv, omc, nkv, nmc);                         \
      t->count   = nc;                                                  \
      release(okv, 0);                                                  \
    }                                                                   \
                                                                        \
    t->kvs       = nkv;                                                 \
    t->max_count = nmc;                                                 \
  }                                                                     \
                                                                        \
    bool t##_get(T* t, K k, V* v) {                                     \
    bool out;                                                           \
                                                                        \
    if ( t->kvs == NULL )                                               \
      out = false;                                                      \
    else {                                                              \
      T##KV* kv = t##_find(t, k, hashf(k));                             \
      out       = kv->key != NK;                                        \
                                                                        \
      if ( out && v )                                                   \
        *v = kv->val;                                                   \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool t##_set(T* t, K k, V v) {                                        \
    if ( check_grow(t) )                                                \
      grow_##t(t);                                                      \
                                                                        \
    T##KV* kv = t##_find(t, k, hashf(k));                               \
    bool out  = kv->key == NK;                                          \
                                                                        \
    if ( out )                                                          \
      kv->key = k;                                                      \
    if ( kv->val == NV )                                                \
      t->count++;                                                       \
    kv->val = v;                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool t##_del( T* t, K k, V* v ) {                                     \
    bool out;                                                           \
                                                                        \
    if ( t->kvs == NULL )                                               \
      out = false;                                                      \
    else {                                                              \
      T##KV* kv = t##_find(t, k, hashf(k));                             \
      out    = kv->key != NK;                                           \
                                                                        \
      if ( out ) {                                                      \
        kv->key = NK;                                                   \
        if ( v )                                                        \
          *v = kv->val;                                                 \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  V t##_intern(T* t, K k, T##InternFn ifn) {                            \
    if ( check_grow(t) )                                                \
      grow_##t(t);                                                      \
    hash_t h = hashf(k);                                                \
    T##KV* kv = t##_find(t, k, h);                                      \
                                                                        \
    if ( kv->key == NK ) {                                              \
      if ( kv->val == NV )                                              \
        t->count++;                                                     \
      ifn(t, kv, (void*)k, h);                                          \
    }                                                                   \
                                                                        \
    return kv->val;                                                     \
  }

bool cmp_strings( char * sx, char* sy ) {
  return strcmp(sx, sy) == 0;
}

hash_t rehash_string(StringsKV* kv) {
  Str* s = kv->val;
  return s->hash;
}

TABLE_IMPL(Strings, char*, Str*, strings, NULL, NULL, hash_string, rehash_string, cmp_strings);

bool cmp_symbols( Sym* sx, Sym* sy ) {
  return sx->val == sy->val;
}

hash_t hash_symbol( Sym* sx ) {
  return sx->hash;
}

hash_t rehash_symbol( EMapKV* kv ) {
  Sym* s = kv->key;

  return s->hash;
}

TABLE_IMPL(EMap, Sym*, int, emap, NULL, -1, hash_symbol, rehash_symbol, cmp_symbols );
