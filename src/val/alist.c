#include <string.h>

#include "val/alist.h"
#include "val/sequence.h"
#include "val/text.h"

#include "lang/compare.h"
#include "lang/print.h"

#include "vm/heap.h"

#include "util/hash.h"
#include "util/number.h"
#include "util/bits.h"

/* Globals */
#define TL_CNT    64ul
#define TL_SIZE   (TL_CNT*sizeof(Val))
#define TL_MASK   63ul
#define VEC_SHIFT  6ul

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

// VTable objects and interface declarations
// Alist
void trace_alist(State* vm, void* x);
void free_alist(State* vm, void* x);
void clone_alist(State* vm, void* x);

VTable AlistVt = {
  .code   =T_ALIST,
  .name   ="Alist",
  .obsize =sizeof(Alist),
  .tracefn=trace_alist,
  .freefn =free_alist,
  .clonefn=clone_alist,
};

// Vec
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

// VNode
void trace_vnode(State* vm, void* x);
void free_vnode(State* vm, void* x);
void clone_vnode(State* vm, void* x);
void seal_vnode(State* vm, void* x, bool d);
void vnode_sinit(Seq* s);
Val  vnode_sfirst(Seq* s);
void vnode_srest(Seq* s);

VTable VNodeVt = {
  .code    =T_VNODE,
  .name    ="VNode",
  .obsize  =sizeof(VNode),
  .is_iseq =true,
  .tracefn =trace_vnode,
  .freefn  =free_vnode,
  .clonefn =clone_vnode,
  .sealfn  =seal_vnode,
  .sinitfn =vnode_sinit,
  .sfirstfn=vnode_sfirst,
  .srestfn =vnode_srest,
};

/* Internal APIs */
// Alist APIs
static bool check_grow(size64 n, size64 c) {
  return n >= c;
}

static bool check_shrink(size64 n, size64 c) {
  return c == MIN_ARR ? n == 0 : n < (c >> 1);
}

static size64 calc_alist_size(size64 n) {
  size64 p = ceil2(n);

  return max(p, MIN_ARR);
}

// Vec/VNode APIs
// size and layout helpers
static inline size32 vec_shft(Vec* v) {
  // maximum shift for this vector
  return v->rt ? v->rt->shft : 0;
}

static inline size64 tl_off(Vec* v) {
  // indices greater than this value are in the tail
  return v->cnt < TL_CNT ? 0 : (v->cnt - 1) & ~TL_MASK;
}

static inline size64 tl_size(Vec* v) {
  // number of elements in the tail
  return v->cnt < TL_CNT ? v->cnt : ((v->cnt - 1) & TL_MASK) + 1;
}

static inline size64 rt_size(Vec* v) {
  // number of elements in the root
  return v->cnt - tl_size(v);
}

static inline bool spc_in_tl(Vec* v) {
  return tl_size(v) < TL_CNT;
}

static inline bool spc_in_rt(Vec* v) {
  // Since ctz is guaranteed to be 64 or less the % shouldn't be too much of a
  // problem (who is this comment for?)
  return ctz(rt_size(v)) % 6 == 0;
}

static inline bool is_last_leaf(Vec* v) {
  /* Used to detect whether popping a leaf will reduce the level of the vector
     will occur when the count is exactly 64 more than a power of 64. */
  size64 rs = rt_size(v);
  return rs == TL_CNT || ctz(rs - TL_CNT) % 6 == 0;
}

static inline size64 idx_for(size64 i, size64 s) {
  return i >> s & TL_MASK;
}

static Val* arr_for(Vec* v, size64 i) {
  if ( i > tl_off(v) )
    return v->tl;

  VNode* n = v->rt;

  for ( size64 l=vec_shft(v); l > 0; l -= VEC_SHIFT )
    n = n->cn[idx_for(i, l)];

  return n->vs;
}

static VNode* last_child(VNode* n) {
  assert(n->cnt > 0);
  assert(n->shft > 0);

  return n->cn[n->cnt-1];
}

static VNode* last_leaf(VNode* n) {
  while ( n->shft > 0 )
    n = last_child(n);

  return n;
}

static void init_vec(Vec* v) {
  v->cnt  = 0;
  v->rt   = NULL;
  v->tl   = rl_alloc(NULL, TL_SIZE);
}

static Vec* new_vec(void) {
  // create and initialize vector
  Vec* v = new_obj(&Vm, T_VEC, 0); init_vec(v);

  // return new vector
  return v;
}

static void init_vnode(VNode* n) {
  n->cnt  = 0;
  n->shft = 0;
  n->full = false;
  n->vs   = rl_alloc(NULL, TL_SIZE);
}

static VNode* new_vnode(void) {
  // create and initialize vnode
  VNode* n = new_obj(&Vm, T_VEC, 0); init_vnode(n);

  // return new vnode
  return n;
}

static VNode* mk_vleaf(Val* vs) {
  // create and initialize new leaf
  VNode* n = new_vnode();
  n->cnt   = TL_CNT;
  n->shft  = 0;
  n->full  = true;

  memcpy(n->vs, vs, TL_SIZE);

  // return the leaf
  return n;
}

static VNode* mk_vnode(void* d, size32 c, size32 s) {
  VNode* n = new_vnode();
  n->cnt   = c;
  n->shft  = s;
  n->full  = false;

  if ( d ) {
    memcpy(n->cn, d, c*sizeof(Val*));
    n->full = c == TL_CNT && (s == 0 || ((VNode**)d)[c-1]->full);
  }

  return n;
}

static VNode* vnode_set(VNode* n, size64 i, Val x) {
  if ( n->sealed ) { // unseal, save, set, and seal
    n = unseal_obj(&Vm, n); preserve(&Vm, 1, tag(n));
    n = vnode_set(n, i, x);
    n = seal_obj(&Vm, n, false);
  } else if ( n->shft == 0 ) {        // leaf, insert at appropriate index
    n->vs[i & TL_MASK] = x;
  } else {                            // insert in appropriate child
    size64 idx = idx_for(i, n->shft);
    VNode* c   = n->cn[idx];
    n->cn[idx] = vnode_set(c, i, x);
  }

  return n;
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

    memcpy(v->tl, lf->vs, TL_SIZE);
}

static VNode* push_leaf(VNode* n, Val* vs) {
  if ( n->sealed ) {
    n = unseal_obj(&Vm, n); preserve(&Vm, 1, tag(n));
    n = push_leaf(n, vs);
    n = seal_obj(&Vm, n, false);

  } else if ( n->shft == VEC_SHIFT ) {
    assert(!n->full);   // should be checked before insertion at child

    // add leaf and update full
    n->cn[n->cnt++] = mk_vleaf(vs);

  } else if ( n->cnt == 0 || n->cn[n->cnt-1]->full ) {
    // create new empty level and add there
    n->cn[n->cnt++] = mk_vnode(NULL, 0, n->shft-VEC_SHIFT);
    push_leaf(n->cn[n->cnt-1], vs);
  } else
    n->cn[n->cnt-1] = push_leaf(n->cn[n->cnt-1], vs);

  // check whether the current node is now full
  n->full = n->cnt == TL_CNT && n->cn[n->cnt-1]->full;

  return n;
}

static VNode* pop_leaf(VNode* n, VNode** lf) {
  /**
   * the control is a little goofy here because we want to avoid making
   * unnecessary copies that won't get used. So we check if a node will be used
   * after popping (possibly calling pop_leaf recursively) before deciding whether
   * the current node needs to be copied preserved. This leads to a lot of duplication
   * of the "unseal, modify, seal" sequence that we normally try to do once up front,
   * but it's actually a lot simpler that way.
   **/

  if ( n->shft == VEC_SHIFT ) {
    *lf = last_child(n);

    if ( n->cnt == 1 )
      n = NULL;

    else if ( n->sealed ) {
      // create a copy to modify
      n = unseal_obj(&Vm, n);
      n->cnt--;
      n = seal_obj(&Vm, n, false);
    } else {
      n->cnt--;
    }
  } else {
    VNode* cn = pop_leaf(last_child(n), lf);

    if ( cn == NULL ) {
      if ( n->cnt == 1 )
        n = NULL;
      else if ( n->sealed ) {
        n = unseal_obj(&Vm, n);
        n->cnt--;
        n = seal_obj(&Vm, n, false);
      } else {
        n->cnt--;
      }
    } else if ( n->sealed ) {
      preserve(&Vm, 1, tag(cn));
      n = unseal_obj(&Vm, n);
      n->cn[n->cnt-1] = cn;
      n = seal_obj(&Vm, n, false);
    } else {
      n->cn[n->cnt-1] = cn;
    }
  }

  return n;
}

static void push_tail(Vec* v) {
  push_leaf(v->rt, v->tl);
  memset(v->tl, 0, TL_CNT*sizeof(Val));
}

static void pop_tail(Vec* v) {
  VNode* lf;

  v->rt = pop_leaf(v->rt, &lf);

  memcpy(v->tl, lf->vs, TL_SIZE);
}

static void add_to_tail(Vec* v, Val x) {
  // current size is equal to next free index
  size64 o = tl_size(v);
  v->tl[o] = x;
  v->cnt++;
}

/* Runtime APIs and interfaces */
// for Alist
void trace_alist(State* vm, void* x) {
  Alist* a = x;

  mark_vals(vm, a->cnt, a->data);
}

void free_alist(State* vm, void* x) {
  (void)vm;

  Alist* a = x;

  // free array
  rl_dealloc(NULL, a->data, 0);

  // re-initialize
  init_alist(a);
}

// for Vec
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
Seq* vec_sinit(Seq* s, void* x) {
  Vec* v = x;

  if ( v->rt ) {
    preserve(&Vm, 1, tag(s));
    s->cseq = mk_seq(v->rt, s->sealed);
  }

  return s;
}

Val  vnode_sfirst(Seq* s);
Seq* vnode_srest(Seq* s);

Val vec_sfirst(Seq* s) {
  Vec* v = s->src;

  if ( s->cseq )
    return vnode_sfirst(s->cseq);

  return v->tl[s->off];
}

Seq* vec_srest(Seq* s) {
  if ( s->sealed ) {
    s = unseal_obj(&Vm, s); preserve(&Vm, 1, tag(s));
    s = vec_srest(s);
    s = seal_obj(&Vm, s, false);
  } else {
    
  }
  
  return s;
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

static bool egal_vnodes(VNode* x, VNode* y) {
  /* only called when two vectors are known to have the same length, so the vnodes
     are assumed to have the same structure. */
  bool r = true;

  if ( x->shft > 0 ) {
    for ( size64 i=0; r && i < TL_CNT; i++ )
      r = rl_egal(x->vs[i], y->vs[i]);
  } else {
    for ( size64 i=0; r && i < x->cnt; i++ )
      r = egal_vnodes(x->cn[i], y->cn[i]);
  }

  return r;
}

static bool egal_vtails(Vec* vx, Vec* vy) {
  size64 ts = tl_size(vx);
  bool r = true;

  for ( size64 i=0; r && i < ts; i++ )
    r = rl_egal(vx->tl[i], vy->tl[i]);

  return r;
}

bool egal_vecs(Val x, Val y) {
  Vec* vx = as_vec(x), * vy = as_vec(y);

  bool r  = vx->cnt == vy->cnt;

  if ( r && vx->cnt > 0 ) {
    if ( vx->rt ) {
      r = egal_vnodes(vx->rt, vy->rt);

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
size64 pr_vnode_vals(State* vm, Port* p, VNode* n) {
  size64 o = 0;

  if ( n ) {
    if ( n->shft == 0 ) {
      for ( size64 i=0; i < TL_CNT; i++ ) {
        o += rl_pr(p, n->vs[i]);
        o += rl_putc(p, ' ');
      }
    } else {
      for ( size64 i=0; i < n->cnt; i++ ) {
        o += pr_vnode_vals(vm, p, n->cn[i]);
      }
    }
  }

  return o;
}

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

// for VNode
void trace_vnode(State* vm, void* x) {
  VNode* n = x;

  if ( n->shft == 0 )
    mark_vals(vm, n->cnt, n->vs);

  else
    mark_objs(vm, n->cnt, n->cn);
}

void free_vnode(State* vm, void* x) {
  (void)vm;

  VNode* n = x;

  rl_dealloc(NULL, n->vs, 0);
}

void clone_vnode(State* vm, void* x) {
  VNode* n = x;

  n->cn = rl_dup(vm, n->cn, TL_SIZE);
}

void seal_vnode(State* vm, void* x, bool d) {
  VNode* n = x;

  if ( d && n->shft > 0 )
    for ( size64 i=0; i < n->cnt; i++ )
      seal_obj(vm, n->cn[i], d);
}

// sequence interface
void vnode_sinit(Seq* s) {
  VNode* n = s->src;
  
  if ( n->shft )
    s->cseq = mk_seq(n->cn[0], s->sealed);
}

Val vnode_sfirst(Seq* s) {
  if ( s->fst == NOTHING ) {
    if ( s->cseq )
      s->fst = vnode_sfirst(s->cseq);

    else { // should be a leaf node
      VNode* n = s->src;
      s->fst   = n->vs[s->off];
    }
  }

  return s->fst;
}

/* External APIs */
Alist* new_alist(void) {
  Alist* a = new_obj(&Vm, T_ALIST, 0);

  init_alist(a);

  return a;
}

void init_alist(Alist* a) {
  a->cnt  = 0;
  a->cap  = 0;
  a->data = NULL;
}

void grow_alist(Alist* a, size64 n) {
  size64 newc = calc_alist_size(n);
  size64 news = newc * sizeof(Val);
  size64 olds = a->cap * sizeof(Val);
  Val*   newd = rl_realloc(NULL, a->data, olds, news);
  a->cap      = newc;
  a->data     = newd;
}

void shrink_alist(Alist* a, size64 n) {
  if ( n == 0 )
    free_alist(&Vm, a);

  else {
    // compute, realloc, and set
    size64 newc = calc_alist_size(n);
    Val*   newd = rl_realloc(NULL, a->data, a->cap*sizeof(Val), newc*sizeof(Val));

    a->cap  = newc;
    a->data = newd;
  }
}

void resize_alist(Alist* a, size64 n) {
  if ( check_grow(n, a->cap) )
    grow_alist(a, n);

  else if ( check_shrink(n, a->cap) )
    shrink_alist(a, n);
}

// external methods
Alist* mk_alist(size64 n, Val* d) {
  Alist* a = new_alist();

  if ( d ) {
    grow_alist(a, n);
    memcpy(a->data, d, n*sizeof(Val));
  }

  return a;
}

Val alist_ref(Alist* a, size64 n) {
  assert(a->cnt < n);

  return a->data[n];
}

size64 alist_add(Alist* a, Val x) {
  if ( check_grow(a->cnt+1, a->cap) )
    grow_alist(a, a->cnt+1);

  size64 i   = a->cnt++;
  a->data[i] = x;

  return i;
}

size64 alist_wrt(Alist* a, size64 n, Val* d) {
  // otherwise buffer may point to invalid memory
  assert(n > 0);

  if ( check_grow(a->cnt+n, a->cap) )
    grow_alist(a, a->cnt+n);

  size64 o = a->cnt;

  if ( d ) {
    memcpy(a->data, d, n*sizeof(Val));
    a->cnt += n;
  }

  return o;
}

void alist_set(Alist* v, size64 n, Val x) {
  assert(n < v->cnt);
  v->data[n] = x;
}

Val alist_pop(Alist* a) {
  assert(a->cnt > 0);

  size64 o = --a->cnt;
  Val r = a->data[o];

  if ( check_shrink(o, a->cap) )
    shrink_alist(a, o);

  return r;
}

Val alist_popn(Alist* a, size64 n, bool e) {
  assert(n <= a->cnt);

  Val r   = a->data[e ? a->cnt-1 : a->cnt-n];
  a->cnt -= n;

  if ( check_shrink(a->cnt, a->cap) )
    shrink_alist(a, a->cnt);

  return r;
}

void alist_cat(Alist* x, Alist* y) {
  if ( y->cnt == 0 )
    return;

  alist_wrt(x, y->cnt, y->data);
}

// Vec/Vnode API
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

// initialization
void rl_toplevel_init_array(void) {
  // comput empty vector hash
  EmptyVec.hash = mix_hashes(hash_word(T_VEC, true),
                             hash_pointer(&EmptyVec, true),
                             true);
}
