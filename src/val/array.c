#include <string.h>

#include "val/array.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/hash.h"
#include "util/number.h"

/* C types */
typedef VNode VLeaf;

/* Globals */
#define TAIL_SIZE   64
#define LEVEL_SHIFT  6
#define MAX_SHIFT   48
#define TAIL_MASK  63ul
#define ROOT_MASK   (~TAIL_MASK)

/* lifetime and comparison methods */
hash_t hash_vec(Val x);
bool   egal_vecs(Val x, Val y);
int    order_vecs(Val x, Val y);

/* Forward declarations for internal APIs */
// internal vector APIs
Vec*  new_vec(size_t n, Val* d, bool t, bool p);
void  init_vec(Vec* v, bool t, bool p);
Vec*  clone_vec(Vec* v);
Vec*  transient_vec(Vec* r);
Vec*  persistent_vec(Vec* r);
void  unpack_vec(Vec* v, MVec* m);
void  resize_vec_tail(Vec* v, size_t c);
void  grow_vec_tail(Vec* v);
void  shrink_vec_tail(Vec* v);
void  add_to_vec(Vec* v, Val x);
void  add_to_tail(Vec* v, Val x);
void  add_to_root(Vec* v, Val* t);
void  set_vec_ref(Vec* v, size_t n, Val x);
void  pop_from_vec(Vec* v, Val* r);

// internal vector node APIs
VNode* new_vnode(size_t s, size_t n, void* d, bool t);
VLeaf* new_vleaf(Val* d, bool t);
VNode* clone_vnode(VNode* n);
VNode* transient_vnode(VNode* n);
VNode* persistent_vnode(VNode* n);
void   unpack_vnode(VNode* n, MVec* m);
void   resize_vnode(VNode* n, size_t c);
void   grow_vnode(VNode* n);
void   shrink_vnode(VNode* n);
VNode* push_tail(VNode* n, Val* t);
VNode* push_to_node(VNode* n, Val* t);
void   add_vleaf(VNode** p, size_t s, Val* t);
void   set_vnode_ref(VNode** b, VNode* n, size_t i, Val x);
VNode* pop_from_vnode(VNode* n, VLeaf** l);

// push/pop helpers

// miscellaneous utilities
bool   space_in_tail(Vec* v);
bool   space_in_root(Vec* v);
size_t sub_index(size_t s, size_t n);
size_t root_count(Vec* v);
size_t tail_count(Vec* v);
size_t tail_size(Vec* v);
size_t tail_offset(Vec* v);
Val*   array_for(Vec* v, size_t n);

/* Internal utilities */
#define transient(x)                            \
  generic((x),                                  \
          Vec*:transient_vec,                \
          VNode*:transient_vnode)(x)

#define persistent(x)                           \
  generic((x),                                  \
          Vec*:persistent_vec,               \
          VNode*:persistent_vnode)(x)

#define unpack(x, a)                            \
  generic((x),                                  \
          Vec*:unpack_vec,                   \
          VNode*:unpack_vnode)(x, a)

/* Internal APIs */
// lifetime and comparison methods
extern hash_t hash_nul(Val x);
extern hash_t get_type_hash(Type* xt);

hash_t hash_vec(Val x) {
  Vec* v = as_vec(x);
  hash_t out = 0;

  if ( v->count == 0 )
    out = hash_nul(NUL);

  else if ( v->packed || v->count < TAIL_SIZE ) {
    out = hash_word(v->count);

    for ( size_t i=0; i<v->count; i++ )
      out = mix_hashes(out, rl_hash(v->tail[i], false));

  } else {
    out = hash_word(v->count);

    MVec buffer = {
      .type     =&MVecType,
      .algo     =true
    };

    unpack(v, &buffer);

    for ( size_t i=0; i<buffer.count; i++ )
      out = mix_hashes(out, rl_hash(buffer.data[i], false));

    free_mvec(&buffer);
  }

  out = mix_hashes(out, get_type_hash(&VecType));

  return out;
}

bool egal_vecs(Val x, Val y) {
  Vec* vx = as_vec(x), * vy = as_vec(y);

  bool out = vx->count == vy->count;

  if ( out ) {
    MVec bx = {}, by = {};

    Val* ax, * ay;

    if ( vx->packed || vx->count < TAIL_SIZE )
      ax = vx->tail;

    else {
      unpack(vx, &bx);
      ax = bx.data;
    }

    if ( vy->packed || vy->count < TAIL_SIZE )
      ay = vy->tail;

    else {
      unpack(vy, &by);
      ay = by.data;
    }

    for ( size_t i=0; out && i<vx->count; i++ )
      out = rl_egal(ax[i], ay[i]);

    if ( bx.data )
      free_mvec(&bx);

    if ( by.data )
      free_mvec(&by);
  }

  return out;
}

int order_vecs(Val x, Val y) {
  Vec* vx = as_vec(x), * vy = as_vec(y);

  int out;

  if ( vx->count == 0 )
    out = 0 - (vy->count > 0);

  else if ( vy->count == 0 )
    out = 1;

  else {
    size_t maxc = min(vx->count, vy->count);

    MVec bx = {}, by = {};

    Val* ax, * ay;

    if ( vx->packed || vx->count < TAIL_SIZE )
      ax = vx->tail;
    
    else {
      unpack(vx, &bx);
      ax = bx.data;
    }
    
    if ( vy->packed || vy->count < TAIL_SIZE )
      ay = vy->tail;
    
    else {
      unpack(vy, &by);
      ay = by.data;
    }

    for ( size_t i=0; out && i<maxc; i++ )
      out = rl_order(ax[i], ay[i]);

    if ( out == 0 )
      out = 0 - (vx->count < vy->count) + (vx->count > vy->count);

    if ( bx.data )
      free_mvec(&bx);
    
    if ( by.data )
      free_mvec(&by);
  }
  
  return out;
}

// vector internals
Vec* new_vec(size_t n, Val* d, bool t, bool p) {
  Vec* o = new_obj(&VecType);
  init_vec(o, t, p);

  if ( p || n <= TAIL_SIZE ) {
    o->tail  = allocate(n * sizeof(Val), false);
    o->count = n;

    if ( d )
      memcpy(o->tail, d, n*sizeof(Val));

  } else {
    assert(d != NULL);
    preserve(1, tag(o));

    o->trans = true;

    for ( size_t i=0; i<n; i++ )
      add_to_vec(o, d[i]);

    if ( !t )
      o = persistent(o);
  }

  return o;
}

void init_vec(Vec* v, bool t, bool p) {
  v->root   = NULL;
  v->trans  = t;
  v->packed = p;
  v->count  = 0;
  v->shift  = 0;
  v->tail   = NULL;
}

Vec* clone_vec(Vec* v) {
  Vec* o = duplicate(v, size_of(v, true), true);
  o->tail = duplicate(o->tail, tail_size(o), false);

  return o;
}

Vec* persistent_vec(Vec* v) {
  if ( v->trans ) {
    v->trans = false;

    if ( v->root )
      persistent(v->root);
  }

  return v;
}

Vec* transient_vec(Vec* v) {
  if ( !v->trans ) {
    v = clone_vec(v);
    v->trans = true;
    v->hash = 0;    // invalidate cached hashes
  }

  return v;
}

void unpack_vec(Vec* v, MVec* m) {
  init_mvec(m, NULL, 0, false, RESIZE_EXACT);
  grow_mvec(m, v->count);

  if ( v->root )
    unpack(v->root, m);

  write_mvec(m, v->tail, tail_count(v));
}

void resize_vec_tail(Vec* v, size_t c) {
  assert(c <= TAIL_SIZE || v->packed);
  assert(v->trans);

  size_t o = tail_size(v);
  size_t n = c * sizeof(Val);
  v->tail  = reallocate(v->tail, o, n, false);
}

void grow_vec_tail(Vec* v) {
  resize_vec_tail(v, tail_count(v) + 1);
}

void shrink_vec_tail(Vec* v) {
  resize_vec_tail(v, tail_count(v) - 1);
}

void add_to_vec(Vec* v, Val x) {
  assert(v->trans);

  if ( !space_in_tail(v) ) {
    add_to_root(v, v->tail);
    resize_vec_tail(v, 0);
  }

  add_to_tail(v, x);
  v->count++;
}

void add_to_tail(Vec* v, Val x) {
  grow_vec_tail(v);

  v->tail[tail_count(v)] = x;
}

void add_to_root(Vec* v, Val* t) {
  if ( v->root == NULL ) {
    VNode* r       = new_vnode(LEVEL_SHIFT, 1, NULL, true);
    v->root        = r;
    r->children[0] = new_vleaf(t, false);
    v->shift       = LEVEL_SHIFT;
  } else if ( !space_in_root(v) ) {
    VNode* r       = new_vnode(v->shift+LEVEL_SHIFT, 2, NULL, true);
    r->children[0] = v->root;
    v->root        = r;

    add_vleaf(&r->children[1], v->shift, t);

    v->shift      += LEVEL_SHIFT;
  } else {
    v->root        = push_tail(v->root, t);
  }
}

void set_vec_ref(Vec* v, size_t n, Val x) {
  
  if ( v->packed )
    v->tail[n] = x;

  else if ( n >= tail_offset(v) )
    v->tail[n & TAIL_MASK] = x;

  else {
    preserve(1, tag(v));
    set_vnode_ref(&v->root, v->root, n, x);
  }
}

void pop_from_vec(Vec* v, Val* r) {
  Val x = v->tail[tail_count(v)-1];

  if ( r )
    *r = x;

  if ( tail_count(v) == 1 && v->root ) { // popped last item, get new tail from root
    VLeaf* l;

    v->root = pop_from_vnode(v->root, &l);

    resize_vec_tail(v, TAIL_SIZE);
    memcpy(v->tail, l->slots, TAIL_SIZE*sizeof(Val));

    if ( v->root ) // may have been removed
      v->shift = v->root->shift;

    else
      v->shift = 0;
  } else {
    shrink_vec_tail(v);
  }

  v->count--;
}

/* Internal VNode APIs. */
VNode* new_vnode(size_t s, size_t n, void* d, bool t) {
  VNode* o = new_obj(&VNodeType);

  o->trans = t;
  o->shift = s;
  o->count = n;
  
  if ( s == 0 ) {
    assert(n == TAIL_SIZE);
    assert(d);

    o->slots = allocate(n*sizeof(Val), false);
    memcpy(o->slots, d, n*sizeof(Val));
  } else {
    assert(n > 0 && n <= TAIL_SIZE);
    o->children = allocate(n*sizeof(VNode*), false);
    
    if ( d )
      memcpy(o->children, d, n*sizeof(VNode*));
  }

  return o;
}

VNode* new_vleaf(Val* d, bool t) {
  return new_vnode(0, TAIL_SIZE, d, t);
}

VNode* clone_vnode(VNode* n) {
  VNode* out = duplicate(n, size_of(n, true), true);
  out->children = duplicate(n->children, n->count*sizeof(VNode*), false);
  return out;
}

VNode* transient_vnode(VNode* n) {
  if ( !n->trans ) {
    n = clone_vnode(n);
    n->trans = true;
  }

  return n;
}

VNode* persistent_vnode(VNode* n) {
  if ( n->trans ) {
    n->trans = false;

    if ( n->shift > 0 )
      for ( size_t i=0; i<n->count; i++ )
        persistent(n->children[i]);
  }

  return n;
}

void unpack_vnode(VNode* n, MVec* m) {
  if ( n->shift == 0 )
    write_mvec(m, n->slots, n->count);

  else
    for ( size_t i=0; i<n->count; i++ )
      unpack(n->children[i], m);
}

void resize_vnode(VNode* n, size_t c) {
  assert(c > 0 && c <= TAIL_SIZE);
  assert(n->shift > 0);
  assert(n->trans);

  n->children = reallocate(n->children, n->count*sizeof(VNode*), c*sizeof(VNode*), false);
  n->count = c;
}

VNode* push_tail(VNode* n, Val* t) {
  VNode* o, *c;
  size_t lc = n->count;
  size_t s = n->shift;

  if ( s == LEVEL_SHIFT ) {
    if ( lc == TAIL_SIZE )     // No more space, return NULL to signal new level needs to be created
      o = NULL;

    else {
      o = n->trans ? n : transient(n);
      preserve(1, tag(o));
      lc = o->count;
      grow_vnode(o);
      o->children[lc] = new_vleaf(t, false);
    }
  } else {
    lc = n->count;
    c = n->children[lc-1]; // try to push to last child
    c = push_tail(c, t);

    if ( c == NULL ) {
      if ( lc == TAIL_SIZE ) // No more spacde, return NULL to signal new level needs to be created
        o = NULL;

      else {
        o = n->trans ? n : transient(n);
        preserve(1, tag(o));
        grow_vnode(o);
        add_vleaf(&o->children[lc], s-LEVEL_SHIFT, t);
      }
    }
  }

  return o;
}

void add_vleaf(VNode** p, size_t s, Val* t) {
  if ( s == 0 )
    *p = new_vleaf(t, false);

  else {
    VNode* n = *p = new_vnode(s, 1, NULL, true);
    add_vleaf(&n->children[0], s-LEVEL_SHIFT, t);
  }
}

void set_vnode_ref(VNode** b, VNode* n, size_t i, Val x) {
  if ( !n->trans )
    n = *b = transient(n);

  if ( n->shift == 0 )
    n->slots[i & TAIL_MASK] = x;

  else {
    size_t subi = sub_index(n->shift, i);
    set_vnode_ref(&n->children[subi], n->children[subi], i, x);
  }
}

VNode* pop_from_vnode(VNode* n, VLeaf** l) {
  VNode* o, * c;

  if ( n->shift == 0 ) {
     o = NULL;
    *l = n;
  } else {
    
  }

  return o;
}


/* Misc Vec utilities. */
bool space_in_tail(Vec* v) {
  return tail_count(v) < TAIL_SIZE;
}

bool space_in_root(Vec* v) {
  return root_count(v) < (1ul << (v->shift + LEVEL_SHIFT));
}

size_t tail_count(Vec* v) {
  size_t out;

  if ( v->packed )
    out = v->count;

  else if ( v->count & TAIL_MASK )
    out = v->count & TAIL_MASK;

  else
    out = TAIL_SIZE;

  return out;
}

size_t sub_index(size_t s, size_t n) {
  return n >> s & TAIL_MASK;
}

size_t root_count(Vec* v) {
  size_t out;

  if ( v->packed ) // no root
    out = 0;

  else if ( v->count & TAIL_MASK ) // tail is not full
    out = v->count & ROOT_MASK;

  else // tail is full
    out = v->count - TAIL_SIZE;

  return out;
}

size_t tail_size(Vec* v) {
  return tail_count(v) * sizeof(Val);
}

size_t tail_offset(Vec* v) {
  if ( v->count < TAIL_SIZE )
    return 0;

  return ((v->count - 1) >> LEVEL_SHIFT) << LEVEL_SHIFT;
}

Val* array_for(Vec* v, size_t i) {
  if ( i >= tail_offset(v) )
    return v->tail;

  VNode* n = v->root;

  for (size_t l = v->shift; l > 0; l -= LEVEL_SHIFT )
    n = n->children[(i >> l) & TAIL_MASK];

  return n->slots;
}

/* External APIs */
/* Vec API */
Vec* mk_vec(size_t n, Val* d) {
  return new_vec(n, d, false, false);
}

Vec* packed_vec(size_t n, Val* d) {
  return new_vec(n, d, false, true);
}

Val vec_ref(Vec* v, size_t n) {
  assert(n < v->count); // should already be checked

  Val out;

  if ( v->packed )
    out = v->tail[n];

  else {
    Val* a = array_for(v, n);
    out = a[n & LEVEL_SHIFT];
  }

  return out;
}

Vec* vec_add(Vec* v, Val x) {
  if ( !v->trans ) {
    v = transient(v);
    add_to_vec(v, x);
    v = persistent(v);
  } else {
    add_to_vec(v, x);
  }

  return v;
}

Vec* vec_set(Vec* v, size_t n, Val x) {
  assert(n < v->count); // should already be checked
  
  if ( !v->trans ) {
    v = transient(v);
    set_vec_ref(v, n, x);
    v = persistent(v);
  } else {
    set_vec_ref(v, n, x);
  }

  return v;
}

Vec* vec_pop(Vec* v, Val* r) {
  assert(v->count > 0); // should already be checked
  
  if ( !v->trans ) {
    v = transient(v);
    preserve(1, tag(v));
    pop_from_vec(v, r);
    v = persistent(v);
  } else {
    pop_from_vec(v, r);
  }

  return v;
}

Vec* vec_cat(Vec* x, Vec* y) {
  Vec* o = x;

  if ( y->count > 0 ) {
    bool t = o->trans;

    if ( !t )
      o = transient(o);

    preserve(1, tag(o));

    MVec b;

    unpack(y, &b);

    for ( size_t i=0; i<b.count; i++ )
      add_to_vec(o, b.data[i]);

    if ( !t )
      o = persistent(o);

    free_mvec(&b);
  }
  
  return o;
}

/* Mutable ARRAY APIs */
#define MUTABLE_ARRAY(T, t, X)                                          \
  T* new_##t(X* d, size_t n, bool s, ResizeAlgorithm ag) {              \
    T* o = new_obj(&T##Type);                                           \
    init_##t(o, NULL, 0, s, ag);                                        \
                                                                        \
    if ( d )                                                            \
      write_##t(o, d, n);                                               \
                                                                        \
    return o;                                                           \
  }                                                                     \
                                                                        \
  void init_##t(T* a, X* _s, size_t ms, bool s, ResizeAlgorithm ag) {   \
    if ( a->type == NULL ) { /* temporary buffer on C stack */          \
      a->type  = &T##Type;                                              \
      a->free  = true;                                                  \
      a->trace = false;                                                 \
      a->sweep = false;                                                 \
    }                                                                   \
                                                                        \
    a->algo = ag;                                                       \
    a->shrink = s;                                                      \
    a->data = _s;                                                       \
    a->_static = _s;                                                    \
    a->max_static = ms;                                                 \
    a->count = 0;                                                       \
    a->max_count = 0;                                                   \
  }                                                                     \
                                                                        \
  void free_##t(T* a) {                                                 \
    if ( a->data && a->data != a->_static )                             \
      deallocate(a->data, 0, false);                                    \
    init_##t(a, a->_static, a->max_static, a->shrink, a->algo);         \
  }                                                                     \
                                                                        \
  void grow_##t(T* a, size_t n) {                                       \
    (void)a;                                                            \
    (void)n;                                                            \
  }                                                                     \
                                                                        \
  void shrink_##t(T* a, size_t n) {                                     \
    (void)a;                                                            \
    (void)n;                                                            \
  }

MUTABLE_ARRAY(MVec, mvec, Val);
MUTABLE_ARRAY(Alist, alist, void*);

#undef MUTABLE_ARRAY
