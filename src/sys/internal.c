// headers --------------------------------------------------------------------
#include <string.h>

#include "sys/internal.h"
#include "sys/memory.h"

#include "data/sym.h"

#include "lang/base.h"

#include "util/hashing.h"

// macros ---------------------------------------------------------------------
// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

#define tcheck_grow(t) ((t)->count >= (t)->max_count*LOADF)

// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
// function prototypes --------------------------------------------------------
/* symbol table */
void init_symbol_table_kvs(STEntry* kvs, lcount_t max_count);
STEntry* symbol_table_find(SymbolTable* st, char* k, hash_t h);
lcount_t rehash_symbol_table(STEntry* old, lcount_t old_max, STEntry* new, lcount_t new_max);

/* itable  */
void init_itable_kvs(TEntry* kvs, lcount_t max_count);
TEntry* itable_find(ITable* it, Expr k, hash_t h);
lcount_t rehash_itable(TEntry* old, lcount_t old_max, TEntry* new, lcount_t new_max);

// function implementations ---------------------------------------------------
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
      syms = &syms[idx];
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
  
}

bool itable_get(ITable* it, Expr k, Expr* b);
bool itable_set(ITable* it, Expr k, Expr v);
bool itable_del(ITable* it, Expr k, Expr* b);
Ref* environ_get(ITable* it);

bool t##_get(T* t, K k, V* v) {                                         \
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




// initialization -------------------------------------------------------------

#undef MIN_CAP
#undef LOADF
