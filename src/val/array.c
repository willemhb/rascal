#include "val/array.h"

/* Globals */
#define TAIL_SIZE   64
#define LEVEL_SHIFT  6
#define MAX_SHIFT   48

/* Forward declarations for lifetime and comparison methods */

/* Forward declarations for internal APIs */
Vector*  new_vec(size_t n, Value* d, bool t);
Vector*  clone_vec(Vector* v);
Vector*  transient_vector(Vector* v);
Vector*  persistent_vector(Vector* v);
void     unpack_vector(Vector* v, MutVec* m);

VecNode* new_vec_node(size_t o, size_t n, void* d, bool t);
VecNode* clone_vec_node(VecNode* n);
VecNode* transient_vec_node(VecNode* n);
VecNode* persistent_vec_node(VecNode* n);
void     unpack_vec_node(VecNode* n, MutVec* m);

size_t   tail_size(Vector* v);
size_t   tail_offset(Vector* v);
Value*   array_for(Vector* v, size_t n);

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
// vector internals
Vector* transient_vector(Vector* v) {
  if ( !v->transient ) {
    v            = clone_vec(v);
    v->transient = true;
  }

  return v;
}

size_t tail_size(Vector* v) {
  
}

size_t tail_offset(Vector* v) {
  if ( v->count < TAIL_SIZE )
    return 0;

  return ((v->count - 1) >> LEVEL_SHIFT) << LEVEL_SHIFT;
}


void unpack_vector(Vector* v, MutVec* m) {
  if ( v->root )
    unpack(v->root, m);

  write_mvec(m, v->tail, tail_size(v));
}


void unpack_vec_node(VecNode* vn, MutVec* m) {
  if ( vn->offset == 0 )
    write_mvec(m, vn->slots, vn->count);

  else
    for ( size_t i=0; i<vn->count; i++ )
      unpack(vn->children[i], m);
}


/* */
