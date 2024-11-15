#include <string.h>

#include "val/array.h"

#include "vm/heap.h"

#include "util/number.h"
#include "util/bits.h"

/* Globals */
#define TL_SIZE   64ul
#define TL_MASK   63ul
#define VEC_SHIFT  6ul

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
  return v->cnt < TL_SIZE ? 0 : (v->cnt - 1) & ~TL_MASK;
}

static inline size64 tl_size(Vec* v) {
  // number of elements in the tail
  return v->cnt < TL_SIZE ? v->cnt : ((v->cnt - 1) & TL_MASK) + 1;
}

static inline size64 rt_size(Vec* v) {
  // number of elements in the root
  return v->cnt - tl_size(v);
}

static inline bool spc_in_tl(Vec* v) {
  return tl_size(v) < TL_SIZE;
}

static inline bool spc_in_rt(Vec* v) {
  // Since ctz is guaranteed to be 64 or less the % shouldn't be too much of a
  // problem (who is this comment for?)
  return ctz(rt_size(v)) % 6 == 0;
}

static inline size64 idx_for(size64 i, size64 s) {
  return i >> s & TL_MASK;
}

static Val* arr_for(Vec* v, size64 i) {
  if ( i < tl_off(v) )
    return v->tl;

  VNode* n = v->rt;

  for ( size64 l=vec_shft(v); l > 0; l -= VEC_SHIFT )
    n = n->cn[idx_for(i, l)];

  return n->vs;
}

static void init_vec(Vec* v) {
  v->cnt  = 0;
  v->rt   = NULL;
  v->tl   = rl_alloc(NULL, TL_SIZE*sizeof(Val));
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
  n->vs   = rl_alloc(NULL, TL_SIZE*sizeof(Val));
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
  n->cnt   = TL_SIZE;

  memcpy(n->vs, vs, TL_SIZE*sizeof(Val));

  // return the leaf
  return n;
}

static VNode* mk_vnode(VNode** cn, size32 c, size32 s) {
  VNode* n = new_vnode();
  n->cnt   = c;
  n->shft  = s;

  if ( cn )
    memcpy(n->cn, cn, c*sizeof(VNode*));

  return n;
}

static void add_vec_level(Vec* v) {
  size64 shft = vec_shft(v);

  if ( shft == 0 )
    v->rt = mk_vnode(NULL, 0, VEC_SHIFT);

  else
    v->rt = mk_vnode(&v->rt, 1, shft + VEC_SHIFT);
}

static bool add_to_vnode(VNode* n, Val* vs) {
  if ( n->cnt == 0 ) {
    
  }
}

static void push_tail(Vec* v) {
  // TODO
}

static void add_to_tail(Vec* v, Val x) {
  // current size is equal to next free index
  size64 o = tl_size(v);
  v->tl[o] = x;
  v->cnt++;
}

/* Runtime APIs */
void trace_alist(State* vm, void* x) {
  Alist* a = x;

  for ( size_t i=0; i < a->cnt; i++ )
    mark(vm, a->data[i]);
}

void free_alist(State* vm, void* x) {
  (void)vm;

  Alist* a = x;

  // free array
  rl_dealloc(NULL, a->data, 0);

  // re-initialize
  init_alist(a);
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
    seal_obj(&Vm, v);
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
    v = unseal_obj(&Vm, v);
    v = vec_add(v, x);
    v = seal_obj(&Vm, v);
  } else {
    if ( !spc_in_tl(v) ) {
      if ( !spc_in_rt(v) )
        add_vec_level(v);

      push_tail(v);
    }

    add_to_tail(v, x);
  }

  return v;
}
