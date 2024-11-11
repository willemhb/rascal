#include <math.h>

#include "val/table.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/number.h"

/* Globals */
#define LF      0.625
#define LF_I    1.6
#define MIN_CAP 8ul

/* Internal APIs */
static inline bool is_ts(TNode* n) {
  return n->key == NOTHING && n->hash != 0;
}

static inline bool isnt_used(TNode* n) {
  return n->key == NOTHING && n->hash == 0;
}

static size64 pad_table_size(size64 n) {
  if ( n == 0 )
    return MIN_CAP;
  
  size64 p = ceil2(ceil(n * LF_I));

  return max(p, MIN_CAP);
}

static bool check_resize(size64 n, size64 c) {
  return c == 0 || n >= ceil(c * LF);
}

static void init_table_kvs(TNode* ns, size64 cap) {
  for (size64 i=0; i<cap; i++)
    ns[i] = (TNode){ NOTHING, 0, NOTHING };
}

static TNode* alloc_table_kvs(size64 cap) {
  TNode* ns = rl_alloc(NULL, cap * sizeof(TNode));

  init_table_kvs(ns, cap);

  return ns;
}

/* Rehash and return an updated count of live entries */
static size64 rehash_table_kvs(TNode* ons, size64 om, TNode* nns, size64 nm) {
  size64 nc = 0, msk = nm - 1;

  for (size64 oi=0; oi < om; oi++ ) {
    TNode* ol = &ons[oi];

    if ( ol->key == NOTHING )
      continue;

    size64 ni = ol->hash & msk;

    while ( nns[ni].key != NOTHING )
      ni = (ni + 1) & msk;

    nns[ni] = *ol;
    nc++;
  }

  return nc;
}

static TNode* find_node(Table* t, Val k, hash64 h) {
  if ( t->kvs == NULL )
    return NULL;

  TNode* ns = t->kvs, *n, *ts = NULL;

  size64 m = t->cap-1, idx = h & m;

  for (;;) {
    n = &ns[idx];

    if ( n->key == NOTHING ) {
      if ( n->hash == 0 ) // empty entry, didn't find key
        break;

      // check if we've found any tombstones yet. If not, save this one
      if ( ts == NULL )
        ts = n;
    }

    // compare hashes first to save a potentially expensive value comparison
    else if ( n->hash == h && rl_egal(k, n->key) )
      break;

    // check next index (return to beginning of table if we overflow)
    idx = ( idx + 1 ) & m;
  }

  return ts ? : n;
}

static void resize_table(Table* t, size64 n) {
  if ( n == 0 )
    free_table(&Vm, t);

  else {
    size64 nm  = pad_table_size(n);
    TNode* nns = alloc_table_kvs(nm);

    if ( t->kvs == NULL )
      t->kvs = nns;

    else {
      // get old table, rehash, and update count
      size64 om  = t->cap;
      TNode* ons = t->kvs;
      size64 nc  = rehash_table_kvs(ons, om, nns, nm);

      // set updated count and kvs
      t->cnt     = nc;
      t->kvs    = nns;

      // free the old table
      rl_dealloc(NULL, ons, 0);
    }

    t->cap = nm;
  }
}

/* Runtime methods */
void trace_table(State* vm, void* x) {
  Table* t = x; TNode* ns = t->kvs;

  for (size64 i=0, m=0; i < t->cap && m < t->cnt; i++ ) {
    TNode* n = &ns[i];

    if ( n->hash ) {
      m++;

      if ( n->key != NOTHING ) {
        mark(vm, n->key);
        mark(vm, n->val);
      }
    }
  }
}

void free_table(State* vm, void* x) {
  (void)vm;

  Table* t = x;

  rl_dealloc(NULL, t->kvs, 0);
}

/* External APIs */
// Table API
Table* mk_table(State* vm) {
  Table* t = new_obj(vm, T_TABLE, 0);

  init_table(vm, t);

  return t;
}

void init_table(State* vm, Table* t) {
  (void)vm;

  t->cnt = 0;
  t->cap = 0;
  t->kvs  = NULL;
}

bool table_get(Table* t, Val k, Val* v) {
  hash64 h = rl_hash(k);
  TNode* n = find_node(t, k, h);
  bool   o = n && n->key != NOTHING;

  if ( o && v )
    *v = n->val;

  return o;
}

bool table_set(Table* t, Val k, Val v) {
  if ( check_resize(t->cnt+1, t->cap) )
    resize_table(t, t->cnt+1);

  hash64 h = rl_hash(k);
  TNode* n = find_node(t, k, h);
  bool   o = n->key == NOTHING;

  if ( o ) {
    n->key = k;

    if ( n->hash == 0 ) // fresh entry, not a tombstone
      t->cnt++;

    n->hash = h;
  }

  n->val = v;

  return o;
}

bool table_add(Table* t, Val k, TNode** l) {
  if ( check_resize(t->cnt+1, t->cap) )
    resize_table(t, t->cnt+1);

  hash64 h = rl_hash(k);
  TNode* n = find_node(t, k, h);
  bool   o = n->key == NOTHING;

  if ( o ) {
    n->key = k;

    if ( n->hash ) // reusing a tombstone
      t->cnt++;

    n->hash = h;
  }

  if ( l )
    *l = n;

  return o;
}

bool table_del(Table* t, Val k, Val* v) {
  hash64 h = rl_hash(k);
  TNode* n = find_node(t, k, h);
  bool   o = n != NULL && n->key != NOTHING;

  if ( o ) {
    /* create a tombstone; invalidate the `key` and `val` fields but leave the hash. */
    n->key = NOTHING;

    if ( v )
      *v = n->val;

    n->val = NOTHING;
  }

  return o;
}

void join_tables(Table* tx, Table* ty) {
  if ( ty->kvs != NULL ) {
    if ( check_resize(tx->cnt+ty->cnt, tx->cap) )
      resize_table(tx, tx->cnt+ty->cnt);

    TNode* ykvs = ty->kvs;
    size64 ycnt = ty->cnt, ymax = ty->cap;

    for ( size64 i=0, cnt=0; i < ymax && cnt < ycnt; i++ ) {
      TNode* ykv = &ykvs[i];

      if ( ykv->hash )
        ycnt++;

      // we're duplicating some of the code from table_set to avoid recomputing the hash
      if ( ykv->key != NOTHING ) {
        TNode* xkv = find_node(tx, ykv->key, ykv->hash);

        if ( xkv->key == NOTHING ) {
          xkv->key = ykv->key;

          if ( xkv->hash != 0 )
            tx->cnt++;

          xkv->hash = ykv->hash;
        }

        xkv->val = ykv->val;
      }
    }
  }
}

#undef LF
