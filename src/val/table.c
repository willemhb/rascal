#include <math.h>
#include <string.h>

#include "val/table.h"
#include "val/list.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/number.h"
#include "util/bits.h"
#include "util/hash.h"

/* Globals */
// table parameters
#define LF      0.625
#define LF_I    1.6
#define MIN_CAP 8ul

// map parameters
#define MN_MAXC  64ul
#define MN_EDGEC 32ul
#define MN_MINC  8ul
#define MN_MSK   63ul
#define MN_SHFT  6ul
#define MAX_SHFT 42ul

// empty map singleton
Map EmptyMap = {
  .tag     = T_MAP,
  .nosweep = true,
  .nofree  = true,
  .sealed  = true,
  .gray    = true,
};

/* Internal APIs */
// internal table APIs
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

// internal Map APIs
// hash segment
static inline size64 mn_aidx(MNode* mn, hash64 h) {
  return h >> mn->shft && MN_MSK;
}

static inline size64 mn_hmsk(MNode* mn, hash64 h) {
  return 1ul << (h >> mn->shft && MN_MSK);
}

static inline size64 mn_cidx(MNode* mn, hash64 h) {
  size64 hm = mn_hmsk(mn, h);

  return popc((mn->bm | hm) & (hm - 1));
}

static inline size64 mn_idx(MNode* mn, hash64 h) {
  return mn->cap == MN_MAXC ? mn_aidx(mn, h) : mn_cidx(mn, h);
}

static inline bool aidx_free(MNode* mn, hash64 h) {
  return (mn->bm & mn_hmsk(mn, h)) == 0;
}

static inline bool cidx_free(MNode* mn, hash64 h) {
  return mn_cidx(mn, h) == mn->cnt;
}

static inline hash64 cd_hash(Obj* o) {
  if ( o->tag == T_LIST )
    o = as_obj(as_list(o)->head);

  return o->hash;
}

static void mn_comp_kvs(MNode* n, Obj** dst) {
  // compress child array (when shrinking from 64 -> 32)
  Obj** src = n->cn;

  for ( size64 i=0, j=0; i < MN_MAXC && j < n->cnt; i++ ) {
    Obj* cd = src[i];

    if ( cd == NULL )
      continue;
    
    dst[j++] = cd;
  }
}

static void mn_decomp_kvs(MNode* mn, Obj** dst) {
  // decompress child array (when growing from 32 -> 64)
  Obj** src = mn->cn;

  for ( size64 i=0; i < mn->cnt; i++ ) {
    Obj* cd  = src[i];
    hash64 h = cd_hash(cd);
    size64 s = mn_aidx(mn, h);
    dst[s]   = cd;
  }
}

static void mn_grow(MNode* n) {
  assert(n->cap < MN_MAXC);

  size64 oc = n->cap;
  size64 nc = oc << 1;
  size64 os = oc * sizeof(Obj*);
  size64 ns = nc * sizeof(Obj*);
  Obj** ocn = n->cn;
  Obj** ncn = rl_alloc(NULL, ns);

  if (nc == MN_MAXC)
    mn_decomp_kvs(n, ncn);

  else
    memcpy(ncn, ocn, os);

  rl_dealloc(NULL, ocn, 0);

  n->cap = nc;
  n->cn  = ncn;
}

static void mn_shrink(MNode* n) {
  size64 oc = n->cap;
  size64 nc = max(oc << 1, MN_MINC);
  size64 ns = nc * sizeof(Obj*);
  Obj** ocn = n->cn;
  Obj** ncn = rl_alloc(NULL, ns);

  if (oc == MN_MAXC)
    mn_comp_kvs(n, ncn);

  else
    memcpy(ncn, ocn, ns);

  rl_dealloc(NULL, ocn, 0);

  n->cap = nc;
  n->cn  = ncn;
}

static void mn_shunt(MNode* n, size64 i) {
  // move everything above index i
  size64 di = i+1;
  size64 c  = n->cnt-i;
  size64 s  = c * sizeof(Obj*);

  memmove(n->cn+di, n->cn+i, s);
}

static void mn_unshunt(MNode* n, size64 i) {
  size64 di = i;
  size64 c  = n->cnt-i-1;
  size64 s  = c * sizeof(Obj*);

  memmove(n->cn+di, n->cn+i+1, s);
}

static size64 mn_setbm(MNode* n, size64 h) {
  // manages updating the bitmap and the count, as well as moving items in the child array if necessary
  if ( n->cnt == n->cap )
    mn_grow(n);

  size64 aidx = mn_aidx(n, h), idx = aidx;

  if ( n->cap < MN_MAXC ) {
    idx = popc((n->bm | aidx) & (aidx - 1));

    if ( idx < n->cnt )
      mn_shunt(n, idx);
  }

  n->cnt += 1;
  n->bm  |= aidx;

  return idx;
}

static void mn_delbm(MNode* n, hash64 h) {
  
}

static void add_leaf(MNode* n, Pair* kv) {
  hash64 h  = kv->hash;
  size64 i  = mn_setbm(n, h);
  n->cn[i]  = (Obj*)kv;
}

static Pair* mn_get(MNode* n, hash64 h) {
  Pair* o = NULL;

  for (;;) {
    size64 idx = mn_idx(n, h);
    Obj* cd    = n->cn[idx];

    if ( cd == NULL )
      break;

    if ( cd->tag == T_MNODE ) { // child node
      n = (MNode*)cd;

    } else if ( cd->tag == T_PAIR ) {
      if ( cd->hash == h )
        o = (Pair*)cd;
      
      break;
    } else { // collision node
      assert(cd->tag == T_LIST);
      List* kvs = (List*)cd;

      while ( o == NULL && kvs->cnt > 0 ) {
        Pair* kv = as_pair(kvs->head);

        if ( kv->hash == h )
          o = kv;

        else
          kvs = kvs->tail;
      }

      break;
    }
  }

  return o;
}

static Pair* mn_put(MNode* mn, MNode** pl, bool rs, hash64 h, Val k) {
  Pair* kv = NULL;

  if ( rs )
    mn = *pl = unseal_obj(&Vm, mn);

  size64 ai = mn_aidx(mn, h);
  size64 ab = 1 << ai;

  if ( (mn->bm & ai) == 0 ) { // abstract index is free, insert new leaf
    kv = mk_kv(k, h);
    add_leaf(mn, kv);

  } else {
    size64 ci = mn->cnt == MN_MAXC ? ai : popc(mn->bm & (ab - 1));
    Obj*   cd = mn->cn[ci];

    if ( )
  }

  if ( rs )
    seal_obj(&Vm, mn);

  return kv;
}

static void do_map_set(Map* m, Val k, Val v, bool rs) {
  hash64 h = rl_hash(k);

  if ( m->root == NULL )
    m->root = new_mnode(rs, MAX_SHFT, 0);

  Pair* kv = mn_put(m->root, &m->root, rs, h, k);

  if ( kv->cdr == NOTHING ) // new key
    m->cnt++;

  kv->cdr = v;

  if ( rs ) {
    seal_obj(&Vm, m);
    seal_obj(&Vm, kv);
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

// Map API
Map* mk_map(size64 n, Val* kvs) {
  Map* r;
  
  if ( n == 0 )
    r = &EmptyMap;

  else {
    assert((n & 1) == 0); // even number of key/value pairs
    
    r = new_map();

    preserve(&Vm, 1, tag(r));

    for ( size64 i=0; i < n; i+= 2 )
      r = map_set(r, kvs[i], kvs[i+1]);

    r = seal_obj(&Vm, r);
  }

  return r;
}

bool map_get(Map* m, Val k, Val* v) {
  bool r = false;

  if ( m->cnt > 0 ) {
    hash64 h = rl_hash(k);
    MNode* n = m->root;
    Pair* kv = mn_get(n, h);

    if ( kv != NULL && rl_egal(k, kv->car) ) {
      r = true;

      if ( v )
        *v = kv->cdr;
    }
  }

  return r;
}

bool map_has(Map* m, Val k) {
  return map_get(m, k, NULL);
}

Map* map_set(Map* m, Val k, Val v) {
  if ( m->sealed ) {
    m = unseal_obj(&Vm, m);
    preserve(&Vm, 1, tag(m));
    do_map_set(m, k, v, true);

  } else {
    do_map_set(m, k, v, false);
  }

  return m;
}

// initialization
void rl_toplevel_init_map(void) {
  // set empty map hash
  EmptyMap.hash = mix_hashes(hash_word(T_MAP), hash_pointer(&EmptyMap));
}

#undef LF
#undef LF_I
#undef MIN_CAP
#undef MN_MAXC
#undef MN_MINC
#undef MN_MSK
#undef MN_SHFT
#undef MAX_SHFT
