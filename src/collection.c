#include <string.h>

#include "collection.h"
#include "runtime.h"
#include "util.h"


// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

// Alist APIs
void init_alist(Alist* a) {
  a->vals      = NULL;
  a->count     = 0;
  a->max_count = 0;
}

void free_alist(Alist* a) {
  release(a->vals, 0);
  init_alist(a);
}

void grow_alist(Alist* a) {
  size_t new_maxc = a->max_count ? a->max_count << 1 : MIN_CAP;
  void** new_spc  = reallocate(false,
                               a->max_count * sizeof(void*),
                               new_maxc * sizeof(void*),
                               a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void shrink_alist(Alist* a) {
  assert(a->max_count > MIN_CAP);

  size_t new_maxc = a->max_count >> 1;
  void*  new_spc  = reallocate(false,
                               a->max_count*sizeof(void*),
                               new_maxc*sizeof(void*),
                               a->vals);

  a->vals      = new_spc;
  a->max_count = new_maxc;
}

void alist_push(Alist* a, void* v) {
  if ( a->count == a->max_count )
    grow_alist(a);

  a->vals[a->count++] = v;
}

void* alist_pop(Alist* a) {
  void* out = a->vals[--a->count];

  if ( a->count == 0 )
    free_alist(a);

  else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) )
    shrink_alist(a);

  return out;
}

// Table implementation
void init_table(Table* t) {
  t->kvs       = NULL;
  t->count     = 0;
  t->max_count = 0;
}

void free_table(Table* t) {
  release(t->kvs, 0);
  init_table(t);
}

#define TABLE_IMPL(K, V, T, type, NK, NV, hashf, rehashf, cmpf)         \
  void   init_##type##_kvs(KV* kvs, size_t max_count) {                 \
    for ( size_t i=0; i < max_count; i++ ) {                            \
      kvs[i].key = (void*)NK;                                           \
      kvs[i].val = (void*)NV;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  KV* type##_find(Table* t, K k) {                                      \
    assert(t->kvs != NULL);                                             \
    KV* kvs = t->kvs, * kv, * ts = NULL;                                \
    size_t msk = t->max_count-1;                                        \
    hash_t hash = hashf(k);                                             \
    size_t idx = hash & msk;                                            \
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
    }                                                                   \
                                                                        \
    return ts ? : kv;                                                   \
  }                                                                     \
                                                                        \
  size_t rehash_##type(KV* old, size_t omc, KV* new, size_t nmc ) {     \
    size_t cnt = 0;                                                     \
                                                                        \
    for ( size_t i=0; i < omc; i++ ) {                                  \
      KV* kv = &old[i];                                                 \
      if ( kv->key == NK )                                              \
        continue;                                                       \
                                                                        \
      cnt++;                                                            \
                                                                        \
      hash_t hash = rehashf(kv);                                        \
      size_t msk  = nmc - 1;                                            \
      size_t idx  = hash & msk;                                         \
      while ( new[idx].key != NK )                                      \
        idx = (idx+1) & msk;                                            \
                                                                        \
      new[idx] = *kv;                                                   \
    }                                                                   \
    return cnt;                                                         \
  }                                                                     \
                                                                        \
  void grow_##type(Table* t) {                                          \
    size_t nmc = t->max_count < MIN_CAP ? MIN_CAP : t->max_count << 1;  \
    KV*    nkv = allocate(false, nmc*sizeof(KV));                       \
                                                                        \
    init_##type##_kvs(nkv, nmc);                                        \
                                                                        \
    if ( t->kvs != NULL ) {                                             \
      size_t omc = t->max_count;                                        \
      KV*    okv = t->kvs;                                              \
      size_t nc  = rehash_##type(okv, omc, nkv, nmc);                   \
      t->count   = nc;                                                  \
      release(okv, 0);                                                  \
    }                                                                   \
                                                                        \
    t->kvs       = nkv;                                                 \
    t->max_count = nmc;                                                 \
  }                                                                     \
                                                                        \
  bool type##_get(Table* t, K k, V* v) {                                \
                                                                        \
  }                                                                     \
    
bool cmp_strings( char * sx, char* sy ) {
  return strcmp(sx, sy) == 0;
}

hash_t rehash_string(KV* kv) {
  return hash_string(kv->key);
}

TABLE_IMPL(char*, Obj*, Strings, strings, NULL, NULL, hash_string, rehash_string, cmp_strings);
