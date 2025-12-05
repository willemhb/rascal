// headers --------------------------------------------------------------------
#include <string.h>

#include "sys/internal.h"
#include "sys/memory.h"
#include "sys/error.h"

#include "data/sym.h"

#include "lang/base.h"

#include "util/hashing.h"
#include "util/number.h"

// macros ---------------------------------------------------------------------
// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

#define tcheck_grow(t)    ((t)->count >= (t)->max_count*LOADF)
#define acheck_grow(a, n) ((n) >= (a)->max_count)
#define bcheck_grow(b, n) ((n) + (b)->encoded == (b)->max_count)

// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
// function prototypes --------------------------------------------------------
/* helpers */
lcount_t calc_array_size(lcount_t n);
lcount_t calc_buffer_size(lcount_t n);

/* symbol table */
void init_symbol_table_kvs(STEntry* kvs, lcount_t max_count);
STEntry* symbol_table_find(SymbolTable* st, char* k, hash_t h);
lcount_t rehash_symbol_table(STEntry* old, lcount_t old_max, STEntry* new, lcount_t new_max);
void grow_symbol_table(SymbolTable* st);

/* itable  */
void init_itable_kvs(TEntry* kvs, lcount_t max_count);
TEntry* itable_find(ITable* it, Expr k, hash_t h);
lcount_t rehash_itable(TEntry* old, lcount_t old_max, TEntry* new, lcount_t new_max);
void grow_itable(ITable* it);

/* ibuffer */
void grow_ibuffer(IBuffer* b);
void shrink_ibuffer(IBuffer* b);
void resize_ibuffer(IBuffer* b, lcount_t n);

// function implementations ---------------------------------------------------
/* helpers */
lcount_t calc_array_size(lcount_t n) {
  if (n == 0)
    return 0;

  else if (n <= MIN_CAP)
    return MIN_CAP;

  else
    return cpow2(n);
}

lcount_t calc_buffer_size(lcount_t n) {
  if (n == 0)
    return 0;

  lcount_t pad = n;

  if (pad <= MIN_CAP)
    return MIN_CAP;

  else
    return cpow2(pad);
}

/* symbol table */
/* internal */
void init_symbol_table_kvs(STEntry* kvs, lcount_t max_count) {
  for ( lcount_t i=0; i < max_count; i++ ) {
    kvs[i].key = NULL;
    kvs[i].val = NULL;
  }
}

STEntry* symbol_table_find(SymbolTable* st, char* k, hash_t h) {
  assert(st->symbols != NULL);
  STEntry* syms = st->symbols, * sym, * ts = NULL;

  uptr_t msk = st->max_count-1;
  lcount_t idx = h & msk;

  for (;;) {
      sym = &syms[idx];
      if ( sym->key == NULL ) {
        if ( sym->val != NULL ) /* tombstone */
          ts = ts ? : sym;
        else
          break;
      } else if ( !strcmp(k, sym->val->val) )
        break;

      idx = (idx + 1) & msk;
    }
    return ts ? : sym;
}

lcount_t rehash_symbol_table(STEntry* old, lcount_t old_max, STEntry* new, lcount_t new_max) {
  lcount_t cnt = 0;

  for ( lcount_t i=0; i < old_max; i++ ) {
    STEntry* kv = &old[i];
    if ( kv->key == NULL ) // tombstone or unused
      continue;

    cnt++;

    hash_t hash = kv->val->hash;
    uptr_t msk  = new_max - 1;
    lcount_t idx  = hash & msk;
    while ( new[idx].key != NULL )
      idx = (idx+1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

void grow_symbol_table(SymbolTable* st) {
  lcount_t nmc = st->max_count < MIN_CAP ? MIN_CAP : st->max_count << 1;
  STEntry* nkv = allocate(NULL, false, nmc*sizeof(STEntry));

  init_symbol_table_kvs(nkv, nmc);

  if ( st->symbols != NULL ) {
    lcount_t omc = st->max_count;
    STEntry* okv = st->symbols;
    lcount_t nc = rehash_symbol_table(okv, omc, nkv, nmc);
    st->count = nc;
    release(NULL, okv, 0);
  }

  st->symbols = nkv;
  st->max_count = nmc;
}

/* external */
void init_symbol_table(SymbolTable* st) {
  reset_symbol_table(st);
}

void reset_symbol_table(SymbolTable* st) {
  st->symbols = NULL;
  st->count = 0;
  st->max_count = 0;
}

void free_symbol_table(SymbolTable* st) {
  release(NULL, st->symbols, 0);
  reset_symbol_table(st);
}

Sym* intern_symbol(SymbolTable* st, char* k) {
  if ( tcheck_grow(st) )
    grow_symbol_table(st);

  hash_t h = hash_string(k);
  STEntry* e = symbol_table_find(st, k, h);

  if ( e->key == NULL ) {
    if ( e->val == NULL )
      st->count++;

    e->val = new_sym(k, true, h);
    e->key = e->val->val;
  }

  return e->val;
}

/* ITable */
/* internal */
void init_itable_kvs(TEntry* kvs, lcount_t max_count) {
  for ( lcount_t i=0; i < max_count; i++ ) {
    kvs[i].key = NOTHING;
    kvs[i].val = NOTHING;
  }
}

TEntry* itable_find(ITable* it, Expr k, hash_t h) {
  assert(it->kvs != NULL);
  TEntry* kvs = it->kvs, * kv, * ts = NULL;

  uptr_t msk = it->max_count-1;
  lcount_t idx = h & msk;

  for (;;) {
      kv = &kvs[idx];
      if ( kv->key == NOTHING ) {
        if ( kv->val != NOTHING ) /* tombstone */
          ts = ts ? : kv;
        else
          break;
      } else if ( egal_exps(k, kv->key) )
        break;

      idx = (idx + 1) & msk;
    }
    return ts ? : kv;
}

lcount_t rehash_itable(TEntry* old, lcount_t old_max, TEntry* new, lcount_t new_max) {
  lcount_t cnt = 0;

  for ( lcount_t i=0; i < old_max; i++ ) {
    TEntry* kv = &old[i];
    if ( kv->key == NOTHING ) // tombstone or unused
      continue;

    cnt++;

    hash_t hash = hash_exp(kv->key);
    uptr_t msk  = new_max - 1;
    lcount_t idx  = hash & msk;
    while ( new[idx].key != NOTHING )
      idx = (idx+1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

void grow_itable(ITable* it) {
  lcount_t nmc = it->max_count < MIN_CAP ? MIN_CAP : it->max_count << 1;
  TEntry* nkv = allocate(NULL, false, nmc*sizeof(TEntry));

  init_itable_kvs(nkv, nmc);

  if ( it->kvs != NULL ) {
    lcount_t omc = it->max_count;
    TEntry* okv = it->kvs;
    lcount_t nc = rehash_itable(okv, omc, nkv, nmc);
    it->count = nc;
    release(NULL, okv, 0);
  }

  it->kvs = nkv;
  it->max_count = nmc;
}


/* external */
void init_itable(ITable* it, bool environ) {
  it->environ = environ;
  reset_itable(it);
}

void reset_itable(ITable* it) {
  it->kvs = NULL;
  it->count = 0;
  it->max_count = 0;
}

void free_itable(ITable* it) {
  release(NULL, it->kvs, 0);
  reset_itable(it);
}

bool itable_get(ITable* it, Expr k, Expr* b) {
    bool out;

    if ( it->kvs == NULL )
      out = false;
    else {
      TEntry* kv = itable_find(it, k, hash_exp(k));
      out = kv->key != NOTHING;

      if ( out && b )
        *b = kv->val;
    }

    return out;
}

bool itable_set(ITable* it, Expr k, Expr v) {
    if ( tcheck_grow(it) )
      grow_itable(it);

    TEntry* kv = itable_find(it, k, hash_exp(k));
    bool out = kv->key == NOTHING;

    if ( out )
      kv->key = k;
    if ( kv->val == NOTHING )
      it->count++;
    kv->val = v;
    return out;
}

bool itable_del(ITable* it, Expr k, Expr* b) {
    bool out;

    if ( it->kvs == NULL )
      out = false;
    else {
      TEntry* kv = itable_find(it, k, hash_exp(k));
      out = kv->key != NOTHING;

      if ( out ) {
        kv->key = NOTHING;
        if ( b )
          *b = kv->val;
      }
    }
    return out;
}

/* ibuffer */
/* internal */
void grow_ibuffer(IBuffer* b) {
  if ( b->max_count == MAX_ARITY )
    runtime_error("maximum array size exceeded");
  
  lcount_t new_maxc = b->max_count ? b->max_count << 1 : MIN_CAP;
  void*  new_spc = reallocate(NULL,
                              false,
                              new_maxc * b->elsize,
                              b->max_count * b->elsize,
                              b->data);
  
  b->data = new_spc;
  b->max_count = new_maxc;
}

void resize_ibuffer(IBuffer* b, lcount_t n) {
  
  if ( b->max_count == MAX_ARITY )
    runtime_error("maximum array size exceeded");
  
  lcount_t new_maxc = calc_buffer_size(n);
  void*  new_spc = reallocate(NULL,
                              false,
                              new_maxc * b->elsize,
                              b->max_count * b->elsize,
                              b->data);
  
  b->data = new_spc;
  b->max_count = new_maxc;
}

/* external */
void init_ibuffer(IBuffer* ib, int elsize, bool encoded) {
  ib->elsize = elsize;
  ib->encoded = encoded;
  reset_ibuffer(ib);
}

void reset_ibuffer(IBuffer* ib) {
  ib->data = NULL;
  ib->count = 0;
  ib->max_count = 0;
}

void free_ibuffer(IBuffer* ib) {
  release(NULL, ib->data, 0);
}

void ibuffer_add8(IBuffer* ib, byte_t d) {
  
}

void ibuffer_add16(IBuffer* ib, ushort_t d);
void ibuffer_write8(IBuffer* ib, lcount_t n, byte_t* d);
void ibuffer_write16(IBuffer* ib, lcount_t n, ushort_t* d);

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
    if ( xs != NULL )                                                   \
      memcpy(a->vals+a->count, xs, n*sizeof(X));                        \
                                                                        \
    a->count += n;                                                      \
  }

// initialization -------------------------------------------------------------

#undef MIN_CAP
#undef LOADF
#undef tcheck_grow
#undef acheck_grow
#undef bcheck_grow
