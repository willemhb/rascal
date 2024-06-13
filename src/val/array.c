#include <string.h>

#include "val/array.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/hash.h"
#include "util/number.h"

/* Globals */
#define TAIL_SIZE   64
#define LEVEL_SHIFT  6
#define MAX_SHIFT   48
#define LEVEL_MASK  63

/* lifetime and comparison methods */
hash_t   hash_vec(Value x);
bool     egal_vecs(Value x, Value y);
int      order_vecs(Value x, Value y);

/* Forward declarations for internal APIs */
// internal vector APIs
Vector*  new_vec(size_t n, Value* d, bool t, bool p);
Vector*  clone_vec(Vector* v);
Vector*  transient_vector(Vector* r);
Vector*  persistent_vector(Vector* r);
void     unpack_vector(Vector* v, MutVec* m);
void     grow_vec_tail(Vector* v, size_t n);
void     shrink_vec_tail(Vector* v, size_t n);
bool     space_in_tail(Vector* v);
void     add_to_tail(Vector* v, Value x);

// internal vector node APIs
VecNode* new_vec_node(size_t s, size_t n, void* d, bool t);
VecNode* new_vec_leaf(Value* d, bool t);
VecNode* clone_vec_node(VecNode* r, VecNode* n);
VecNode* transient_vec_node(VecNode* n);
VecNode* persistent_vec_node(VecNode* n);
void     unpack_vec_node(VecNode* n, MutVec* m);
void     grow_vec_node(VecNode* n, uint32_t c);
void     shrink_vec_node(VecNode* n, uint32_t c);

// push/pop helpers
void push_tail(Vector* v, Value* t);
void pop_tail(Vector* v, VecNode** t);

// miscellaneous utilities
size_t tail_count(Vector* v);
size_t tail_size(Vector* v);
size_t tail_offset(Vector* v);
Value* array_for(Vector* v, size_t n);

/* Internal utilities */
#define transient(x)                            \
  generic((x),                                  \
          Vector*:transient_vector,             \
          VecNode*:transient_vec_node)(x)

#define persistent(x)                           \
  generic((x),                                  \
          Vector*:persistent_vector,            \
          VecNode*:persistent_vec_node)(x)

#define unpack(x, a)                            \
  generic((x),                                  \
          Vector*:unpack_vector,                \
          VecNode*:unpack_vec_node)(x, a)

/* Internal APIs */
// lifetime and comparison methods
extern hash_t hash_nul(Value x);
extern hash_t get_type_hash(Type* xt);

hash_t hash_vec(Value x) {
  Vector* v = as_vec(x);
  hash_t out = 0;

  if ( v->count == 0 )
    out = hash_nul(NUL);

  else if ( v->packed || v->count < TAIL_SIZE ) {
    out = hash_word(v->count);

    for ( size_t i=0; i<v->count; i++ )
      out = mix_hashes(out, rl_hash(v->tail[i], false));

  } else {
    out = hash_word(v->count);

    MutVec buffer = {
      .type     =&MutVecType,
      .algo     =true
    };

    unpack(v, &buffer);

    for ( size_t i=0; i<buffer.count; i++ )
      out = mix_hashes(out, rl_hash(buffer.data[i], false));

    free_mvec(&buffer);
  }

  out = mix_hashes(out, get_type_hash(&VectorType));

  return out;
}

bool egal_vecs(Value x, Value y) {
  Vector* vx = as_vec(x), * vy = as_vec(y);

  bool out = vx->count == vy->count;

  if ( out ) {
    MutVec bx = {}, by = {};

    Value* ax, * ay;

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

int order_vecs(Value x, Value y) {
  Vector* vx = as_vec(x), * vy = as_vec(y);

  int out;

  if ( vx->count == 0 )
    out = 0 - (vy->count > 0);

  else if ( vy->count == 0 )
    out = 1;

  else {
    size_t maxc = min(vx->count, vy->count);

    MutVec bx = {}, by = {};

    Value* ax, * ay;
    
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
Vector* new_vec(size_t n, Value* d, bool t, bool p) {
  Vector* v = new_obj(&VectorType);
  v->root = NULL;
  v->packed = p;
  v->trans = true;
  v->count = n;

  if ( p ) {
      v->tail = allocate(n*sizeof(Value), false);

      if ( d != NULL )
        memcpy(v->tail, d, tail_size(v));

  } else {
    preserve(1, tag(v));
    size_t e = tail_offset(v), s = tail_size(v), i;

    for ( i=0; i < e; i+= TAIL_SIZE )
      push_tail(v, d+i);

    v->tail = allocate(s, false);
    memcpy(v->tail, d+i, s);
  }

  if ( !t )
    persistent(v);

  return v;
}

Vector* clone_vec(Vector* v) {
  preserve(1, tag(v));

  Vector* o = duplicate(v, size_of(v, true), true);
  o->tail = duplicate(o->tail, tail_size(o), false);

  return o;
}

Vector* persistent_vector(Vector* v) {
  if ( v->trans ) {
    v->trans = false;

    if ( v->root )
      persistent(v->root);
  }

  return v;
}

void unpack_vector(Vector* v, MutVec* m) {
  init_mvec(m, NULL, 0, true, true, false);
  grow_mvec(m, v->count);

  if ( v->root )
    unpack(v->root, m);
  
  write_mvec(m, v->tail, tail_count(v));
}

Vector* transient_vector(Vector* v) {
  if ( !v->trans ) {
    v = clone_vec(v);
    v->trans = true;
    v->hash = 0;    // invalidate cached hashes
  }

  return v;
}

/* Internal VecNode APIs. */
VecNode* new_vec_node(size_t s, size_t n, void* d, bool t);

VecNode* new_vec_leaf(Value* d, bool t) {
  return new_vec_node(0, TAIL_SIZE, d, t);
}

VecNode* clone_vec_node(VecNode* r, VecNode* n);
VecNode* transient_vec_node(VecNode* n);
VecNode* persistent_vec_node(VecNode* n);

void unpack_vec_node(VecNode* n, MutVec* m) {
  if ( n->shift == 0 )
    write_mvec(m, n->slots, n->count);

  else
    for ( size_t i=0; i<n->count; i++ )
      unpack(n->children[i], m);
}

void grow_vec_node(VecNode* n, uint32_t c) {
  assert( c <= TAIL_SIZE );
  assert( n->shift > 0 );

  uint32_t m = ceil2(c);

  if ( m > n->max_count ) {
    size_t old = n->max_count*sizeof(Object*);
    size_t new = m*sizeof(Object*);
    n->children = reallocate(n->children, old, new, false);
    n->max_count = m;
  }

  n->count = c;
}

void shrink_vec_node(VecNode* n, uint32_t c);

// Vector push/pop utilities
bool space_in_tail(Vector* v) {
  return tail_size(v) < TAIL_SIZE;
}

void add_to_vec(Vector* v, Value x) {
  if ( space_in_tail(v) ) {
    
  }
}

VecNode* add_leaf(size_t shift, Value* t) {
  VecNode* out = new_vec_node(shift, 1, NULL, true), * parent = out, * child;
  preserve(1, tag(out));

  for ( shift=shift-LEVEL_SHIFT; shift > 0; shift -= LEVEL_SHIFT ) {
    child = new_vec_node(shift, 1, NULL, true);
    parent->children[0] = child;
    parent = child;
  }

  parent->children[0] = new_vec_leaf(t, true);

  return out;
}

void push_tail(Vector* v, Value* t) {
  
}


/* Misc Vector utilities. */
size_t tail_count(Vector* v) {
  size_t out = v->count;

  if ( !v->packed )
    out &= LEVEL_MASK;
  
  return out;
}

size_t tail_size(Vector* v) {
  return tail_count(v) * sizeof(Value);
}

size_t tail_offset(Vector* v) {
  if ( v->count < TAIL_SIZE )
    return 0;

  return ((v->count - 1) >> LEVEL_SHIFT) << LEVEL_SHIFT;
}

Value* array_for(Vector* v, size_t i) {
  if ( i >= tail_offset(v) )
    return v->tail;

  VecNode* n = v->root;

  for (size_t l = v->shift; l > 0; l -= LEVEL_SHIFT )
    n = n->children[(i >> l) & LEVEL_MASK];

  return n->slots;
}

/* External APIs */
Vector* mk_vec(size_t n, Value* d) {
  return new_vec(n, d, false, false);
}

Vector* packed_vec(size_t n, Value* d) {
  return new_vec(n, d, false, true);
}

Value vec_ref(Vector* v, size_t n) {
  Value x;
  
  if ( v->packed ) { // flat array (used internally)
    x = v->tail[n];
    } else {
    Value* arr = array_for(v, n);
    x = arr[n & LEVEL_MASK];
  }

  return x;
}

Vector* vec_add(Vector* v, Value x) {
  if ( !v->trans ) {
    v = transient(v);
    v = vec_add(v, x);
    v = persistent(v);

  } else {
    add_to_tail(v, x);
    v->count++;

    if ( !v->packed && tail_count(v) == TAIL_SIZE ) {
      push_tail(v, v->tail);
      resize_vec_tail(v, 0);
    }
  }

  return v;
}

Vector* vec_set(Vector* v, size_t n, Value x) {
  if ( !v->trans ) {
    v = transient(v);
    v = vec_set(v, n, x);
    v = persistent(v);
  } else {
    
  }

  return v;
}

Vector* vec_cat(Vector* x, Vector* y) {
  if ( !x->trans ) {
    x = transient(x);
    x = vec_cat(x, y);
    x = persistent(x);
  } else {
    
  }

  return x;
}
