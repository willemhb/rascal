#include <string.h>

#include "val/vec.h"
#include "val/vnode.h"
#include "val/seq.h"

#include "vm/heap.h"

#include "lang/compare.h"

#include "util/amt.h"
#include "util/hash.h"
#include "util/bits.h"

/* C types */
/* Forward declarations */
// Helpers
static size32 vec_shft(Vec* v);
static size64 tl_off(Vec* v);
static size64 tl_size(Vec* v);
static size64 rt_size(Vec* v);
static bool spc_in_tl(Vec* v);
static bool spc_in_rt(Vec* v);
static bool is_last_leaf(Vec* v);
static void init_vec(Vec* v);
static Vec* new_vec(void);
static void add_vec_lvl(Vec* v);
static void rm_vec_lvl(Vec* v);
static void push_tail(Vec* v);
static void pop_tail(Vec* v);
static bool egal_vtails(Vec* vx, Vec* vy);

// Interfaces
void   trace_vec(State* vm, void* x);
void   free_vec(State* vm, void* x);
void   clone_vec(State* vm, void* x);
void   seal_vec(State* vm, void* x, bool d);
size64 pr_vec(State* vm, Port* p, Val x);
hash64 hash_vec(Val x);
bool   egal_vecs(Val x, Val y);
int    order_vecs(Val x, Val y);
bool   vec_empty(void* x);
Val    vec_first(void* x);
void   vec_sinit(Seq* s);
Val    vec_sfirst(Seq* s);
void   vec_srest(Seq* s);

// External
Val  vnode_sfirst(Seq* s);
Seq* vnode_srest(Seq* s);

/* Globals */
Vec EmptyVec = {
  .tag     = T_VEC,
  .nosweep = true,
  .nofree  = true,
  .sealed  = true,
  .gray    = true,
  .cnt     = 0,
  .rt      = NULL,
  .tl      = NULL,
};

VTable VecVt = {
  .code    =T_VEC,
  .name    ="Vec",
  .obsize  =sizeof(Vec),
  .is_iseq =true,
  .tracefn =trace_vec,
  .freefn  =free_vec,
  .clonefn =clone_vec,
  .sealfn  =seal_vec,
  .prfn    =pr_vec,
  .hashfn  =hash_vec,
  .egalfn  =egal_vecs,
  .orderfn =order_vecs,
  .emptyfn =vec_empty,
  .firstfn =vec_first,
  .sinitfn =vec_sinit,
  .sfirstfn=vec_sfirst,
  .srestfn =vec_srest,
};

/* Helpers */
static size32 vec_shft(Vec* v) {
  // maximum shift for this vector
  return v->rt ? v->rt->shft : 0;
}

static size64 tl_off(Vec* v) {
  // indices greater than this value are in the tail
  return v->cnt < HT_MAXC ? 0 : (v->cnt - 1) & ~HT_MASK;
}

static size64 tl_size(Vec* v) {
  // number of elements in the tail
  return v->cnt < HT_MAXC ? v->cnt : ((v->cnt - 1) & HT_MASK) + 1;
}

static size64 rt_size(Vec* v) {
  // number of elements in the root
  return v->cnt - tl_size(v);
}

static bool spc_in_tl(Vec* v) {
  return tl_size(v) < HT_MAXC;
}

static bool spc_in_rt(Vec* v) {
  // Since ctz is guaranteed to be 64 or less the % shouldn't be too much of a
  // problem (who is this comment for?)
  return ctz(rt_size(v)) % 6 == 0;
}

static bool is_last_leaf(Vec* v) {
  /* Used to detect whether popping a leaf will reduce the level of the vector
     will occur when the count is exactly 64 more than a power of 64. */
  size64 rs = rt_size(v);
  return rs == HT_MAXC || ctz(rs - HT_MAXC) % 6 == 0;
}

static Val* arr_for(Vec* v, size64 i) {
  if ( i > tl_off(v) )
    return v->tl;

  VNode* n = v->rt;

  for ( size64 l=vec_shft(v); l > 0; l -= HT_SHFT )
    n = n->cn[ht_index_for(i, l)];

  return n->vs;
}

static void init_vec(Vec* v) {
  v->cnt  = 0;
  v->rt   = NULL;
  v->tl   = rl_alloc(NULL, HT_MAXS);
}

static Vec* new_vec(void) {
  // create and initialize vector
  Vec* v = new_obj(&Vm, T_VEC, 0); init_vec(v);

  // return new vector
  return v;
}

static void add_vec_lvl(Vec* v) {
  size64 shft = vec_shft(v);

  if ( shft == 0 )
    v->rt = mk_vnode(NULL, 0, VEC_SHIFT);

  else
    v->rt = mk_vnode(&v->rt, 1, shft + VEC_SHIFT);
}

static void rm_vec_lvl(Vec* v) {
    VNode* lf = last_leaf(v->rt);

    if ( vec_shft(v) == VEC_SHIFT )
      v->rt = NULL;

    else
      v->rt = v->rt->cn[0];

    memcpy(v->tl, lf->vs, HT_SIZE);
}

static void push_tail(Vec* v) {
  push_leaf(v->rt, v->tl);
  memset(v->tl, 0, HT_CNT*sizeof(Val));
}

static void pop_tail(Vec* v) {
  VNode* lf;

  v->rt = pop_leaf(v->rt, &lf);

  memcpy(v->tl, lf->vs, HT_SIZE);
}

static void add_to_tail(Vec* v, Val x) {
  // current size is equal to next free index
  size64 o = tl_size(v);
  v->tl[o] = x;
  v->cnt++;
}

static bool egal_vtails(Vec* vx, Vec* vy) {
  size64 ts = tl_size(vx);
  bool r = true;

  for ( size64 i=0; r && i < ts; i++ )
    r = rl_egal(vx->tl[i], vy->tl[i]);

  return r;
}

/* Interfaces */
void trace_vec(State* vm, void* x) {
  Vec* v = x;

  mark(vm, v->rt);
  mark_vals(vm, tl_size(v), v->tl);
}

void free_vec(State* vm, void* x) {
  (void)vm;

  Vec* v = x;

  rl_dealloc(NULL, v->tl, 0);
}

void clone_vec(State* vm, void* x) {
  Vec* v = x;
  v->tl  = rl_dup(vm, v->tl, tl_size(v)*sizeof(Val));
}

void seal_vec(State* vm, void* x, bool d) {
  Vec* v = x;

  if ( d && v->rt )
    seal_obj(vm, v->rt, d);
}

// sequence interface
void vec_sinit(Seq* s) {
  Vec* v = s->src;

  if ( v->rt ) {
    preserve(&Vm, 1, tag(s));
    s->cseq = mk_seq(v->rt, s->sealed);
  }
}

Val  vnode_sfirst(Seq* s);
Seq* vnode_srest(Seq* s);

Val vec_sfirst(Seq* s) {
  Vec* v = s->src;

  if ( s->cseq )
    return vnode_sfirst(s->cseq);

  return v->tl[s->off];
}

void vec_srest(Seq* s) {
  if ( s->sealed ) {
    s = unseal_obj(&Vm, s);
    preserve(&Vm, 1, tag(s));
    vec_srest(s);
    seal_obj(&Vm, s, false);
  } else {
    
  }
}

// comparison interface
hash64 hash_vec(Val x) {
  Vec* v = as_vec(x);

  assert(v->cnt != 0); // should never be called on empty vector

  Seq* s   = rl_iter(v); preserve(&Vm, 1, tag(s));
  hash64 h = 0;

  while ( !rl_done(s) ) {
    Val x     = rl_first(s);
    hash64 xh = rl_hash(x);
    h         = mix_hashes(h, xh);
  }

  return h;
}

bool egal_vecs(Val x, Val y) {
  Vec* vx = as_vec(x), * vy = as_vec(y);

  bool r  = vx->cnt == vy->cnt;

  if ( r && vx->cnt > 0 ) {
    if ( vx->rt ) {
      r = vn_egal_traverse(vx->rt, vy->rt);

      if ( r )
        r = egal_vtails(vx, vy);

    } else {
      r = egal_vtails(vx, vy);
    }
  }

  return r;
}

int order_vecs(Val x, Val y) {
  Vec* vx = as_vec(x), * vy = as_vec(y);
  int o = 0;

  // reserve space to save sequence objects
  preserve(&Vm, 2, NUL, NUL);
  Seq* sx = rl_iter(vx); add_to_preserved(tag(sx), 0);
  Seq* sy = rl_iter(vy); add_to_preserved(tag(sy), 1);

  while ( !rl_done(sx) && !rl_done(sy) ) {
    Val x = rl_first(sx);
    Val y = rl_first(sy);
    o     = rl_order(x, y);

    if ( o != 0 )
      break;

    else {
      rl_next(sx);
      rl_next(sy);
    }
  }

  if ( o == 0 && vx->cnt != vy->cnt )
    o = 0 - (vx->cnt < vy->cnt) + (vy->cnt > vx->cnt);

  return o;
}

// print
size64 pr_vec(State* vm, Port* p, Val x) {
  (void)vm;

  Vec* v = as_vec(x);
  size64 ts = tl_size(v);
  size64 o = rl_putc(p, '[');
  o += pr_vnode_vals(vm, p, v->rt);

  for ( size64 i=0; i < ts; i++ ) {
    o += rl_pr(p, v->tl[i]);

    if ( i+1 < ts )
      o += rl_putc(p, ' ');
  }

  o += rl_putc(p, ']');

  return o;
}


/* API */
Vec* mk_vec(size64 n, Val* vs) {
  Vec* v = NULL;

  if ( n == 0 )
    v = &EmptyVec;

  else {
    v = new_vec();

    /* preserve for GC, since adding could create new objects and trigger the
       collector. */

    preserve(&Vm, 1, tag(v));

    for ( size64 i=0; i < n; i++ )
      v = vec_add(v, vs[i]);

    // mark persistent
    seal_obj(&Vm, v, true);
  }

  return v;
}

Val vec_ref(Vec* v, size64 n) {
  assert(n < v->cnt);

  Val* a = arr_for(v, n);

  return a[n & TL_MASK];
}

Vec* vec_add(Vec* v, Val x) {
  if ( v->sealed ) {
    v = unseal_obj(&Vm, v); preserve(&Vm, 1, tag(v));
    v = vec_add(v, x);
    v = seal_obj(&Vm, v);
  } else {
    if ( !spc_in_tl(v) ) {
      if ( !spc_in_rt(v) )
        add_vec_lvl(v);

      push_tail(v);
    }

    add_to_tail(v, x);
  }

  return v;
}

Vec* vec_set(Vec* v, size64 i, Val x) {
  if ( v->sealed ) {
    v = unseal_obj(&Vm, v); preserve(&Vm, 1, tag(v));
    v = vec_set(v, i, x);
    v = seal_obj(&Vm, v);
  } else {
    assert(i < v->cnt); // should be checked elsewhere

    if ( i > tl_off(v) )
      v->tl[i & TL_MASK] = x;

    else {
      v->rt = vnode_set(v->rt, i, x);
    }
  }

  return v;
}

Vec* vec_pop(Vec* v, Val* r) {
  assert(v->cnt > 0);

  Val x;

  if ( v->cnt == 1 ) {
    x = v->tl[0];
    v = &EmptyVec;

  } else if ( v->sealed ) {
    v = unseal_obj(&Vm, v); preserve(&Vm, 1, tag(v));
    v = vec_pop(v, r);
    v = seal_obj(&Vm, v, false);  } else {
    size64 tsz = tl_size(v);
    x = v->tl[(tsz -= 1)];
    v->cnt--;

    if ( tsz == 0 ) {
      if ( is_last_leaf(v) )
        rm_vec_lvl(v);

      else
        pop_tail(v);
    }
  }

  if ( r )
    *r = x;

  return v;
}

/* Initialization */
void rl_toplevel_init_vec(void) {
  // comput empty vector hash
  EmptyVec.hash = mix_hashes(hash_word(T_VEC), hash_pointer(&EmptyVec));
}
