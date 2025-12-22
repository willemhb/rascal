#include <string.h>

#include "collection.h"
#include "runtime.h"
#include "util.h"
#include "data.h"

// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

// Exprs implementation -------------------------------------------------------
void init_exprs(RlState* rls, Exprs* a) {
  (void)rls;
  a->vals      = NULL;
  a->count     = 0;
  a->max_count = 0;
}

void free_exprs(RlState* rls, Exprs* a) {
  release(NULL, a->vals, 0);
  init_exprs(rls, a);
}

void grow_exprs(RlState* rls, Exprs* a) {
  if ( a->max_count == MAX_ARITY )
    runtime_error(rls, "maximum exprs size exceeded");

  int new_maxc = a->max_count ? a->max_count << 1 : MIN_CAP;
  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->max_count * sizeof(Expr),
                             a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void shrink_exprs(RlState* rls, Exprs* a) {
  assert(a->max_count > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->max_count >> 1;
  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->max_count * sizeof(Expr),
                             a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void resize_exprs(RlState* rls, Exprs* a, int n) {
  if ( n > MAX_ARITY )
    runtime_error(rls, "maximum exprs size exceeded");

  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->max_count * sizeof(Expr),
                             a->vals);
  a->vals = new_spc;
  a->max_count = new_maxc;
}

void exprs_push(RlState* rls, Exprs* a, Expr x) {
  if ( a->count == a->max_count )
    grow_exprs(rls, a);

  a->vals[a->count++] = x;
}

Expr exprs_pop(RlState* rls, Exprs* a) {
  Expr out = a->vals[--a->count];

  if ( a->count == 0 )
    free_exprs(rls, a);
  else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) )
    shrink_exprs(rls, a);

  return out;
}

void exprs_write(RlState* rls, Exprs* a, Expr* xs, int n) {
  if ( a->count + n > a->max_count )
    resize_exprs(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->vals + a->count, xs, n * sizeof(Expr));

  a->count += n;
}

// Objs implementation --------------------------------------------------------
void init_objs(RlState* rls, Objs* a) {
  (void)rls;
  a->vals      = NULL;
  a->count     = 0;
  a->max_count = 0;
}

void free_objs(RlState* rls, Objs* a) {
  release(NULL, a->vals, 0);
  init_objs(rls, a);
}

void grow_objs(RlState* rls, Objs* a) {
  if ( a->max_count == MAX_ARITY )
    runtime_error(rls, "maximum objs size exceeded");

  int new_maxc = a->max_count ? a->max_count << 1 : MIN_CAP;
  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->max_count * sizeof(void*),
                              a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void shrink_objs(RlState* rls, Objs* a) {
  assert(a->max_count > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->max_count >> 1;
  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->max_count * sizeof(void*),
                              a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void resize_objs(RlState* rls, Objs* a, int n) {
  if ( n > MAX_ARITY )
    runtime_error(rls, "maximum objs size exceeded");

  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->max_count * sizeof(void*),
                              a->vals);
  a->vals = new_spc;
  a->max_count = new_maxc;
}

void objs_push(RlState* rls, Objs* a, void* x) {
  if ( a->count == a->max_count )
    grow_objs(rls, a);

  a->vals[a->count++] = x;
}

void* objs_pop(RlState* rls, Objs* a) {
  void* out = a->vals[--a->count];

  if ( a->count == 0 )
    free_objs(rls, a);
  else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) )
    shrink_objs(rls, a);

  return out;
}

void objs_write(RlState* rls, Objs* a, void** xs, int n) {
  if ( a->count + n > a->max_count )
    resize_objs(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->vals + a->count, xs, n * sizeof(void*));

  a->count += n;
}

// Bin16 implementation -------------------------------------------------------
void init_bin16(RlState* rls, Bin16* a) {
  (void)rls;
  a->vals      = NULL;
  a->count     = 0;
  a->max_count = 0;
}

void free_bin16(RlState* rls, Bin16* a) {
  release(NULL, a->vals, 0);
  init_bin16(rls, a);
}

void grow_bin16(RlState* rls, Bin16* a) {
  if ( a->max_count == MAX_ARITY )
    runtime_error(rls, "maximum bin16 size exceeded");

  int new_maxc = a->max_count ? a->max_count << 1 : MIN_CAP;
  ushort_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(ushort_t),
                                 a->max_count * sizeof(ushort_t),
                                 a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void shrink_bin16(RlState* rls, Bin16* a) {
  assert(a->max_count > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->max_count >> 1;
  ushort_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(ushort_t),
                                 a->max_count * sizeof(ushort_t),
                                 a->vals);

  a->vals = new_spc;
  a->max_count = new_maxc;
}

void resize_bin16(RlState* rls, Bin16* a, int n) {
  if ( n > MAX_ARITY )
    runtime_error(rls, "maximum bin16 size exceeded");

  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  ushort_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(ushort_t),
                                 a->max_count * sizeof(ushort_t),
                                 a->vals);
  a->vals = new_spc;
  a->max_count = new_maxc;
}

void bin16_push(RlState* rls, Bin16* a, ushort_t x) {
  if ( a->count == a->max_count )
    grow_bin16(rls, a);

  a->vals[a->count++] = x;
}

ushort_t bin16_pop(RlState* rls, Bin16* a) {
  ushort_t out = a->vals[--a->count];

  if ( a->count == 0 )
    free_bin16(rls, a);
  else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) )
    shrink_bin16(rls, a);

  return out;
}

void bin16_write(RlState* rls, Bin16* a, ushort_t* xs, int n) {
  if ( a->count + n > a->max_count )
    resize_bin16(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->vals + a->count, xs, n * sizeof(ushort_t));

  a->count += n;
}

// Strings table implementation -----------------------------------------------
#define check_grow(t) ((t)->count >= ((t)->max_count * LOADF))

static void init_strings_kvs(StringsKV* kvs, int max_count) {
  for ( int i = 0; i < max_count; i++ ) {
    kvs[i].key = NULL;
    kvs[i].val = NULL;
  }
}

static bool cmp_strings(char* sx, char* sy) {
  return strcmp(sx, sy) == 0;
}

static hash_t rehash_string(StringsKV* kv) {
  Str* s = kv->val;
  return s->hash;
}

static StringsKV* strings_find(Strings* t, char* k, hash_t h) {
  assert(t->kvs != NULL);
  StringsKV* kvs = t->kvs;
  StringsKV* kv;
  StringsKV* ts = NULL;
  int msk = t->max_count - 1;
  int idx = h & msk;

  for (;;) {
    kv = &kvs[idx];
    if ( kv->key == NULL ) {
      if ( kv->val != NULL ) // tombstone
        ts = ts ? ts : kv;
      else
        break;
    } else if ( cmp_strings(kv->key, k) )
      break;
    idx = (idx + 1) & msk;
  }

  return ts ? ts : kv;
}

static int rehash_strings(StringsKV* old, int omc, StringsKV* new, int nmc) {
  int cnt = 0;

  for ( int i = 0; i < omc; i++ ) {
    StringsKV* kv = &old[i];
    if ( kv->key == NULL )
      continue;

    cnt++;

    hash_t hash = rehash_string(kv);
    int msk = nmc - 1;
    int idx = hash & msk;
    while ( new[idx].key != NULL )
      idx = (idx + 1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

static void grow_strings(RlState* rls, Strings* t) {
  (void)rls;

  int nmc = t->max_count < MIN_CAP ? MIN_CAP : t->max_count << 1;
  StringsKV* nkv = allocate(NULL, nmc * sizeof(StringsKV));

  init_strings_kvs(nkv, nmc);

  if ( t->kvs != NULL ) {
    int omc = t->max_count;
    StringsKV* okv = t->kvs;
    int nc = rehash_strings(okv, omc, nkv, nmc);
    t->count = nc;
    release(NULL, okv, 0);
  }

  t->kvs = nkv;
  t->max_count = nmc;
}

void init_strings(RlState* rls, Strings* t) {
  (void)rls;
  t->kvs       = NULL;
  t->count     = 0;
  t->max_count = 0;
}

void free_strings(RlState* rls, Strings* t) {
  release(NULL, t->kvs, 0);
  init_strings(rls, t);
}

bool strings_get(RlState* rls, Strings* t, char* k, Str** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    StringsKV* kv = strings_find(t, k, hash_string(k));
    out = kv->key != NULL;

    if ( out && v )
      *v = kv->val;
  }

  return out;
}

bool strings_set(RlState* rls, Strings* t, char* k, Str* v) {
  if ( check_grow(t) )
    grow_strings(rls, t);

  StringsKV* kv = strings_find(t, k, hash_string(k));
  bool out = kv->key == NULL;

  if ( out )
    kv->key = k;
  if ( kv->val == NULL )
    t->count++;
  kv->val = v;
  return out;
}

bool strings_del(RlState* rls, Strings* t, char* k, Str** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    StringsKV* kv = strings_find(t, k, hash_string(k));
    out = kv->key != NULL;

    if ( out ) {
      kv->key = NULL;
      if ( v )
        *v = kv->val;
    }
  }
  return out;
}

Str* strings_intern(RlState* rls, Strings* t, char* k, StringsInternFn ifn) {
  if ( check_grow(t) )
    grow_strings(rls, t);

  hash_t h = hash_string(k);
  StringsKV* kv = strings_find(t, k, h);

  if ( kv->key == NULL ) {
    if ( kv->val == NULL )
      t->count++;
    ifn(rls, t, kv, k, h);
  }

  return kv->val;
}

// EMap table implementation --------------------------------------------------
static void init_emap_kvs(EMapKV* kvs, int max_count) {
  for ( int i = 0; i < max_count; i++ ) {
    kvs[i].key = NULL;
    kvs[i].val = NULL;
  }
}

static bool cmp_symbols(Sym* sx, Sym* sy) {
  return sx->val == sy->val;
}

static hash_t hash_symbol(Sym* sx) {
  return sx->hash;
}

static hash_t rehash_symbol(EMapKV* kv) {
  Sym* s = kv->key;
  return s->hash;
}

static EMapKV* emap_find(EMap* t, Sym* k, hash_t h) {
  assert(t->kvs != NULL);
  EMapKV* kvs = t->kvs;
  EMapKV* kv;
  EMapKV* ts = NULL;
  int msk = t->max_count - 1;
  int idx = h & msk;

  for (;;) {
    kv = &kvs[idx];
    if ( kv->key == NULL ) {
      if ( kv->val != NULL ) // tombstone
        ts = ts ? ts : kv;
      else
        break;
    } else if ( cmp_symbols(kv->key, k) )
      break;
    idx = (idx + 1) & msk;
  }

  return ts ? ts : kv;
}

static int rehash_emap(EMapKV* old, int omc, EMapKV* new, int nmc) {
  int cnt = 0;

  for ( int i = 0; i < omc; i++ ) {
    EMapKV* kv = &old[i];
    if ( kv->key == NULL )
      continue;

    cnt++;

    hash_t hash = rehash_symbol(kv);
    int msk = nmc - 1;
    int idx = hash & msk;
    while ( new[idx].key != NULL )
      idx = (idx + 1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

static void grow_emap(RlState* rls, EMap* t) {
  (void)rls;
  int nmc = t->max_count < MIN_CAP ? MIN_CAP : t->max_count << 1;
  EMapKV* nkv = allocate(NULL, nmc * sizeof(EMapKV));

  init_emap_kvs(nkv, nmc);

  if ( t->kvs != NULL ) {
    int omc = t->max_count;
    EMapKV* okv = t->kvs;
    int nc = rehash_emap(okv, omc, nkv, nmc);
    t->count = nc;
    release(NULL, okv, 0);
  }

  t->kvs = nkv;
  t->max_count = nmc;
}

void init_emap(RlState* rls, EMap* t) {
  (void)rls;
  t->kvs       = NULL;
  t->count     = 0;
  t->max_count = 0;
}

void free_emap(RlState* rls, EMap* t) {
  release(NULL, t->kvs, 0);
  init_emap(rls, t);
}

bool emap_get(RlState* rls, EMap* t, Sym* k, Ref** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    EMapKV* kv = emap_find(t, k, hash_symbol(k));
    out = kv->key != NULL;

    if ( out && v )
      *v = kv->val;
  }

  return out;
}

bool emap_set(RlState* rls, EMap* t, Sym* k, Ref* v) {
  if ( check_grow(t) )
    grow_emap(rls, t);

  EMapKV* kv = emap_find(t, k, hash_symbol(k));
  bool out = kv->key == NULL;

  if ( out )
    kv->key = k;
  if ( kv->val == NULL )
    t->count++;
  kv->val = v;
  return out;
}

bool emap_del(RlState* rls, EMap* t, Sym* k, Ref** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    EMapKV* kv = emap_find(t, k, hash_symbol(k));
    out = kv->key != NULL;

    if ( out ) {
      kv->key = NULL;
      if ( v )
        *v = kv->val;
    }
  }
  return out;
}

Ref* emap_intern(RlState* rls, EMap* t, Sym* k, EMapInternFn ifn) {
  if ( check_grow(t) )
    grow_emap(rls, t);

  hash_t h = hash_symbol(k);
  EMapKV* kv = emap_find(t, k, h);

  if ( kv->key == NULL ) {
    if ( kv->val == NULL )
      t->count++;
    ifn(rls, t, kv, k, h);
  }

  return kv->val;
}
