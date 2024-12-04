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
#define MN_MAXP  48ul // max prefix
#define MN_MAXS  42ul // max shift

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

// internal Map/MNode APIs
static Pair* mk_kv(Val k, hash64 h) {
  Pair* kv = mk_pair(k, NOTHING);
  kv->hash = h;

  return kv;
}

static hash64 get_hash_prefix(size64 shft, hash64 h) {
  hash64 o = 0;

  for ( size64 s=MN_MAXS; s > shft; s -= MN_SHFT )
    o |= h & (MN_MSK << s);

  return o;
}

static void init_mnode(MNode* mn, size64 shft, hash64 h) {
  mn->hash = get_hash_prefix(shft, h);
  mn->cnt  = 0;
  mn->cap  = MN_MINC;
  mn->shft = shft;
  mn->bm   = 0;
  mn->cn   = rl_alloc(NULL, MN_MINC*sizeof(Obj*));
}

static MNode* new_mnode(size64 shft, hash64 h) {
  MNode* n = new_obj(&Vm, T_MNODE, MF_NOHASH);

  init_mnode(n, shft, h);

  return n;
}

static inline bool mn_is_compd(MNode* n) {
  return n->cap < MN_MAXC;
}

static inline size64 mn_aidx(MNode* n, hash64 h) {
  return h >> n->shft & MN_MSK;
}

static inline bool mn_has_aidx(MNode* n, size64 ai) {
  return (n->bm & (1ul << ai)) != 0;
}

static inline size64 mn_cidx(MNode* n, size64 ai) {
  /* get the compressed index for the given hash in a compressed node.
     assumes the abstract index exists and the node is compressed.
     violations of these invariants could cause a SIGSEGV. */
  return popc(n->bm & ((1ul << ai) - 1));
}

static size64 mn_idx(MNode* n, size64  ai) {
  /* Get the index for the given node. Returns the abstract index if
     the node is uncompressed, otherwise the compressed index. Assumes
     the abstract index exists. Violation of this invariant could cause
     a SIGSEGV. */
  
  return mn_is_compd(n) ? mn_cidx(n, ai) : ai;
}

static bool mn_cidx_free(MNode* n, size64 ci) {
  return ci+1 == n->cnt;
}

static void mn_grow(MNode* n) {
  assert(n->cap < MN_MAXC);

  size64 oc  = n->cap;
  size64 nc  = oc << 1;
  size64 os  = oc * sizeof(Obj*);
  size64 ns  = nc * sizeof(Obj*);
  Obj**  ocn = n->cn;
  Obj**  ncn = rl_alloc(NULL, ns);
  
  if ( nc == MN_MAXC ) { // uncompress array
    for ( size64 i=0; i < n->cnt; i++ ) {
      Obj* c   = ocn[i];
      hash64 h = c->hash;
      size64 i = mn_aidx(n, h);
      ncn[i]   = c;
    }
  } else {
    memcpy(ncn, ocn, os);
  }

  rl_dealloc(NULL, ocn, 0);

  n->cap = nc;
  n->cn  = ncn;
}

static void mn_add_child(MNode* n, Obj* c, size64 ai) {
  /* Add the given child to the node. Assumes that there is no abstract collision,
     but there may be a compressed collision that needs to be managed. */
  
  if ( n->cnt == n->cap )
    mn_grow(n);

  size64 af = 1ul << ai;

  // update bitmap
  n->bm  |= af;
  n->cnt += 1;

  if ( !mn_is_compd(n) ) {
    n->cn[ai] = c;
  } else {
    size64 ci = mn_cidx(n, ai);

    if ( !mn_cidx_free(n, ci) ) {
      Obj** dst  = n->cn+ci+1;
      Obj** src  = n->cn+ci;
      size64 cnt = (n->cnt-ci)*sizeof(Obj*);

      memmove(dst, src, cnt);
    }

    n->cn[ci] = c;
  }
}

static Pair* mn_put(MNode* n, MNode** p, bool rs, hash64 h, Val k) {
  Pair* kv = NULL;

  if ( rs && n->sealed )
    n = *p = unseal_obj(&Vm, n);

  size64 ai = mn_aidx(n, h);

  if ( !mn_has_aidx(n, ai) ) { // simplest case, no collision
    kv = mk_kv(k, h);

    mn_add_child(n, (Obj*)kv, ai);
  } else {
    size64 i = mn_idx(n, ai); // get the concrete index
    Obj* c   = n->cn[i];

    if ( c->tag == T_MNODE ) // just insert in child
      kv = mn_put((MNode*)c, (MNode**)&n->cn[i], rs, h, k);

    else {
      assert(c->tag == T_PAIR);

      kv = (Pair*)c;

      if ( kv->hash != h ) {
        /*
         * Create a new level, move colliding node down, insert key in new level.
         * TODO: handle colliding hashes.
         */

        MNode* s = new_mnode(n->shft-MN_SHFT, h);
        n->cn[i] = (Obj*)s;

        mn_add_child(s, c, mn_aidx(s, c->hash));
        kv = mn_put(s, (MNode**)&n->cn[i], rs, h, k);
      }
    }
  }

  // cleanup (re-seal if necessary)
  if ( rs ) {
    seal_obj(&Vm, n, false);
    seal_obj(&Vm, kv, false);
  }

  return kv;
}

static Pair* mn_pop(MNode* n, MNode** p, bool rs, hash64 h, Val k);

static Pair* mn_get(MNode* n, hash64 h) {
  Pair* o = NULL;

  for (;;) {
    size64 ai = mn_aidx(n, h);

    if ( !mn_has_aidx(n, ai) )
      break;

    size64 i  = mn_idx(n, ai);
    Obj* cd   = n->cn[i];

    if ( cd->tag == T_MNODE ) { // child node
      n = (MNode*)cd;

    } else {
      assert(cd->tag == T_PAIR);

      if ( cd->hash == h )
        o = (Pair*)cd;
      
      break;
    }
  }

  return o;
}

static void do_map_set(Map* m, Val k, Val v, bool rs) {
  hash64 h = rl_hash(k);

  if ( m->root == NULL )
    m->root = new_mnode(MN_MAXS, h);

  Pair* kv = mn_put(m->root, &m->root, rs, h, k);

  if ( kv->cdr == NOTHING ) // new key
    m->cnt++;

  kv->cdr = v;

  if ( rs ) {
    seal_obj(&Vm, m, false);
    seal_obj(&Vm, kv, false);
  }
}

static void do_map_pop(Map* m, Val k, bool rs) {
  
}

/* Runtime methods */
// Table APIs
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

// Map APIs
void trace_map(State* vm, void* x) {
  Map* m = x;

  mark(vm, m->root);
}

// MNode APIs
void trace_mnode(State* vm, void* x) {
  MNode* n = x;

  if ( mn_is_compd(n) )
    mark_objs(vm, n->cnt, n->cn);

  else
    mark_objs(vm, n->cap, n->cn);
}

void free_mnode(State* vm, void* x) {
  (void)vm;

  MNode* n = x;

  rl_dealloc(NULL, n->cn, 0);
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
// external methods
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

    r = seal_obj(&Vm, r, true);
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

Map* map_pop(Map* m, Val k) {
  if ( m->cnt > 0 && map_has(m, k) ) {
    if ( m->cnt == 1 ) {
        m = &EmptyMap;

    } else if ( m->sealed ) {
      m = unseal_obj(&Vm, m);
      preserve(&Vm, 1, tag(m));
      do_map_pop(m, k, true);
    } else {
      do_map_pop(m, k, false);
    }
  }

  return m;
}

// internal methods
Map* new_map(void) {
  Map* o  = new_obj(&Vm, T_MAP, 0);
  o->cnt  = 0;
  o->root = NULL;

  return o;
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
#undef MN_MAXS
