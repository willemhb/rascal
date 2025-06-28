// headers --------------------------------------------------------------------
#include "data/table.h"


#include <string.h>

#include "runtime.h"
#include "util.h"
#include "data.h"

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

TABLE_IMPL(EMap, Sym*, Ref*, emap, NULL, NULL, hash_symbol, rehash_symbol, cmp_symbols );
