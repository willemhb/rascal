#include <string.h>

#include "hamt.h"
#include "object.h"
#include "number.h"
#include "text.h"

#include "memory.h"
#include "hashing.h"

// misc -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define SHIFT      0x06
#define MAXN       0x40
#define MAXS       0x2a
#define MASK       0x3f
#define MAXH       0x07
#define MAXL       0x08

// internal type implementations ----------------------------------------------
// node -----------------------------------------------------------------------
void  init_node(void* self, data_type_t* type, usize count, flags fl);
void  trace_node(void* self);

data_type_t NodeType = {
  .type={
    .obj={
      .flags=FROZEN,
      .type =DATA_TYPE,
      .gray =true
    },

    .name="node",
    .idno=NODE
  },

  .size =sizeof(node_t),

  .init   =init_node,
  .trace  =trace_node 
};

void init_node(void* self, data_type_t* type, usize count, flags fl) {
  (void)type;
  (void)fl;

  node_t* n   = self;
  n->length   = count;
  n->height   = 0;
}

void trace_node(void* self) {
  node_t* n = self;

  if (n->shift)
    trace_objects(n->length, (object_t**)n->node);

  else
    trace_values(n->length, n->leaf);
}

node_t* leaf_node(value_t tail[64]);
node_t* array_node(uint16 height, uint16 length, node_t** nodes);

// node buffer type (used for vector traversal) -------------------------------
static uint64    tailoff(vector_t* self);
static vector_t* extend_vector(vector_t* self, usize n);

typedef struct {
  vector_t* srcvec;
  uint      maxh;
  bool      smallp;        // length <= MAXN
  uint8     offsets[MAXL];
  node_t*   levels[MAXL];
} nodebuf_t;

static void     init_nodebuf(nodebuf_t* buf, vector_t* srcvec);
static value_t* get_nb_path_to_leaf(nodebuf_t* buf, usize n);
static uint     add_nb_level(nodebuf_t* buf);
static void     extend_nb_level(nodebuf_t* buf, int h);
static bool     replace_nb_level(nodebuf_t* buf, int h);
static void     clone_nb_level(nodebuf_t* buf, int h);
static void     copy_nb_path(nodebuf_t* buf, int h);
static void     clone_nb_levels(nodebuf_t* buf, int minh, int maxh);
static void     copy_nb_paths(nodebuf_t* buf, int minh, int maxh);
static int      push_tail(nodebuf_t* buf);
static node_t*  pop_leaf(nodebuf_t* buf);

static void init_nodebuf(nodebuf_t* buf, vector_t* srcvec) {
  memset(buf, 0, sizeof(nodebuf_t));
  
  buf->srcvec  = srcvec;
  buf->smallp  = srcvec->root;
  buf->maxh    = buf->smallp ? 0 : srcvec->root->height;
}

static value_t* get_nb_path_to_leaf(nodebuf_t* buf, usize n) {
  value_t* array;

  if (n < tailoff(buf->srcvec)) {
    node_t* node = buf->srcvec->root;

    for (int h=node->height; h > 0; h--) {
      uint shift = node->shift;
      usize ln   = n >> shift & MASK;

      buf->offsets[h] = ln;
      buf->levels[h]  = node;

      if (h)
        node = node->node[ln];
    }

    array = node->leaf;

  } else {
    array = buf->srcvec->tail;
  }

  return array;
}

static uint add_nb_level(nodebuf_t* buf) {
  if (buf->smallp) {
    buf->maxh = 1;
    buf->levels[1]  = array_node(1, 1, NULL);
    buf->offsets[1] = 0;
  } else {
    buf->maxh++;
    assert(buf->maxh <= MAXL);
    node_t* node = array_node(buf->maxh, 2, NULL);
    node->node[0] = buf->srcvec->root;
    buf->levels[buf->maxh]  = node;
    buf->offsets[buf->maxh] = 1;
  }

  return buf->maxh;
}

static bool replace_nb_level(nodebuf_t* buf, int h) {
  node_t* node = buf->levels[h];
  bool result = false;

  if (node->length == MAXN) {
    node = array_node(h, 1, NULL);
    buf->levels[h] = node;
    buf->offsets[h] = 0;
    result = true;
  }

  return result;
}

static void extend_nb_level(nodebuf_t* buf, int h) {
  node_t* node = buf->levels[h];

  if (is_frozen(node))
    clone_nb_level(buf, h);

  node->length++;
  buf->offsets[h]++;
}

static void clone_nb_level(nodebuf_t* buf, int h) {
  node_t* node = buf->levels[h];
  buf->levels[h] = clone_object(node, 0);
}

static void copy_nb_path(nodebuf_t* buf, int h) {
  assert(h > 0);
  buf->levels[h]->node[buf->offsets[h]] = buf->levels[h-1];
}

static void clone_nb_levels(nodebuf_t* buf, int minh, int maxh) {
  for (int h=minh; h <= maxh; h++)
    clone_nb_level(buf, h);
}

static void copy_nb_paths(nodebuf_t* buf, int minh, int maxh) {
  for (int h=minh; h <= maxh; h++)
    copy_nb_path(buf, h);
}

static int push_tail(nodebuf_t* buf) {
  buf->levels[0] = leaf_node(buf->srcvec->tail);
  int h = 1;

  while (h <= (int)buf->maxh && buf->offsets[h] == MAXN) {
    replace_nb_level(buf, h);
    h++;
  }

  if (h > (int)buf->maxh)
    add_nb_level(buf);

  else
    extend_nb_level(buf, h);

  return buf->maxh;
}

static 

// vector iterator type (nodebuf_t plus additional state) ---------------------
typedef struct {
  usize     maxcnt;
  usize     count;
  usize     toff;
  nodebuf_t buffer;
} viter_t;

static value_t init_viter(viter_t* iter, vector_t* srcvec);
static value_t next_vitem(viter_t* iter);
static node_t* next_vleaf(viter_t* buf);
static bool    has_next_vitem(viter_t* buf);

static value_t init_viter(viter_t* iter, vector_t* srcvec) {
  init_nodebuf(&iter->buffer, srcvec);
  iter->maxcnt = srcvec->arity;
  iter->count  = 0;
  iter->toff   = tailoff(srcvec);

  return next_vitem(iter);
}

static value_t next_vitem(viter_t* iter) {
  value_t result;
  
  if (iter->count == iter->maxcnt)
    result = STOPITER;

  else if (iter->buffer.smallp)
    result = iter->buffer.srcvec->tail[iter->count++];

  else {
    uint offset = iter->buffer.offsets[0];

    if (offset == MAXN)
      next_vleaf(iter);

    result = iter->buffer.levels[0]->leaf[iter->buffer.offsets[0]];
  }

  return result;
}

static node_t* next_vleaf(viter_t* iter) {
  uint8*   offsets = iter->buffer.offsets;
  node_t** levels  = iter->buffer.levels;
  int maxh         = iter->buffer.maxh;

  offsets[0] = 0;
  int h;

  // find the first array node that has children which have not been iterated
  for (h=1; h <=maxh; h++) {
    if (offsets[h] < MAXN)
      break;

    else
      offsets[h] = 0; // 
  }

  offsets[h]++;

  for (; h > 0; h--)
    levels[h-1] = levels[h]->node[offsets[h]];

  return levels[0];
}

static bool  has_next_vitem(viter_t* iter) {
  return iter->count < iter->maxcnt;
}

// stencil --------------------------------------------------------------------
void* alloc_stencil(data_type_t* type, usize n, flags fl);
void* clone_stencil(void* self, usize padding);
void  init_stencil(void* self, data_type_t* type, usize n, flags fl);
void  trace_stencil(void* self);
usize size_of_stencil(void* self);

data_type_t StencilType = {
  .type={
    .obj={
      .flags=FROZEN,
      .type =DATA_TYPE,
      .gray =true
    },

    .name="stencil",
    .idno=STENCIL
  },

  .size   =sizeof(stencil_t),

  .alloc  =alloc_stencil,
  .copy   =clone_stencil,
  .init   =init_stencil,
  .trace  =trace_stencil,
  
  .size_of=size_of_stencil
};

// APIs -----------------------------------------------------------------------
// vector ---------------------------------------------------------------------
// globals --------------------------------------------------------------------
vector_t EmptyVector = {
  .obj={
    .flags=FROZEN,
    .type =VECTOR,
    .gray =true
  }
};

// forward declarations for internal methods ----------------------------------
// sacred ---------------------------------------------------------------------
void  print_vector(value_t v, port_t* ios);
uhash hash_vector(void* x);
bool  equal_vectors(void* x,  void* y);
int   compare_vectors(value_t x, value_t y);

// lifetime -------------------------------------------------------------------
void* alloc_vector(data_type_t* type, usize count, flags fl);
void* clone_vector(void* self, usize padding);
void  init_vector(void* self, data_type_t* type, usize count, flags fl);
void  trace_vector(void* self);

// misc -----------------------------------------------------------------------
usize size_of_vector(void* self);

data_type_t VectorType = {
  .type={
    .obj={
      .flags=FROZEN,
      .type =DATA_TYPE,
      .gray =true
    },

    .name="vector",
    .idno=VECTOR
  },

  .size =sizeof(vector_t),

  .print  =print_vector,
  .hash   =hash_vector,
  .equal  =equal_vectors,
  .compare=compare_vectors,

  .alloc  =alloc_vector,
  .copy   =clone_vector,
  .init   =init_vector,
  .trace  =trace_vector,

  .size_of=size_of_vector
};

// APIs -----------------------------------------------------------------------

// utilities ------------------------------------------------------------------
static inline uint64 tailoff(vector_t* self) {
  return self->arity <= MAXN ? 0 : self->arity & ~MASK;
}

static value_t* array_for(vector_t* self, usize n) {
  if (n >= tailoff(self))
    return self->tail;

  node_t* node = self->root;

  for (uint32 shift=node->shift; shift > 0; shift -= SHIFT)
    node = node->node[n >> shift & MASK];

  return node->leaf;
}

// sacred ---------------------------------------------------------------------
void print_vector(value_t v, port_t* ios) {
  rl_putc('[', ios);

  viter_t it;

  for ( value_t x=init_viter(&it, as_vector(v)); x != STOPITER; x=next_vitem(&it) ) {
    rl_print(x, ios);

    if (has_next_vitem(&it))
      rl_putc(' ', ios);
  }

  rl_putc(']', ios);
}

uhash hash_vector(void* ptrx) {
  vector_t* vecx = ptrx;

  uhash hash = VectorType.type.obj.hash;

  if (vecx == &EmptyVector)
    return hash_uword(hash);

  viter_t it;

  for (value_t x=init_viter(&it, vecx); x != STOPITER; x=next_vitem(&it)) {
    uhash xhash = rl_hash(x);
    hash = mix_2_hashes(hash, xhash);
  }

  return hash;
}

bool equal_vectors(void* ptrx, void* ptry) {
  vector_t* vecx = ptrx, * vecy = ptry;

  bool result = true;

  if (vecx->arity != vecy->arity)
    result = false;

  else {
    viter_t itx, ity;
    value_t x, y;

    for ( x = init_viter(&itx, vecx), y = init_viter(&ity, vecy);
          result && x != STOPITER && y != STOPITER;
          x = next_vitem(&itx), y = next_vitem(&ity) )
      result = rl_equal(x, y);
  }

  return result;
}

int compare_vectors(value_t valx, value_t valy) {
  vector_t* vecx = as_vector(valx), * vecy = as_vector(valy);
  value_t x, y;
  viter_t itx, ity;
  int o = 0;

  for ( x = init_viter(&itx, vecx), y = init_viter(&ity, vecy);
        x != STOPITER && y != STOPITER;
        x = next_vitem(&itx), y = next_vitem(&ity) ) {
    o = rl_compare(x, y);

    if (o)
      break;
  }

  return o ? : 0 - (x == STOPITER) + (y == STOPITER);
}

// lifetime -------------------------------------------------------------------
void* allocate_vector(data_type_t* type, usize count, flags fl) {
  usize extra = flagp(fl, FROZEN) ? count & MASK : MAXN;
  
  return allocate(type->size + extra * sizeof(value_t));
}

void* clone_vector(void* ptrx, usize padding) {
  vector_t* vecx = ptrx;

  if (vecx->length + padding > MAXN)
    padding = 0;
  
  return duplicate(ptrx, rl_size_of(ptrx), padding * sizeof(value_t));
}

void trace_vector(void* ptrx) {
  vector_t* vecx = ptrx;

  trace_values(vecx->length, vecx->tail);
  rl_mark(vecx->root);
}

// misc -----------------------------------------------------------------------

// external -------------------------------------------------------------------
value_t vector_get(vector_t* self, usize n) {
  assert(n < self->arity);

  value_t* array = array_for(self, n);

  return array[n & MASK];
}

vector_t* vector_set(vector_t* self, usize n, value_t x) {
  assert(n < self->arity);
  vector_t* result = self;
  value_t* array;

  if (has_flag(self, FROZEN)) {
    result = clone_object(self, 0);

    if (n >= tailoff(result))
      array = result->tail;

    else {
      nodebuf_t buf;

      init_nodebuf(&buf, self);
      get_nb_path_to_leaf(&buf, n);
      clone_nb_levels(&buf, 0, buf.maxh);
      copy_nb_paths(&buf, 1, buf.maxh);
      result->root = buf.levels[buf.maxh];
      array = buf.levels[0]->leaf;
    }
  } else
    array = array_for(self, n);

  array[n & MASK] = x;
  return result;
}

vector_t* vector_add(vector_t* self, value_t x) {
  if (self == &EmptyVector)
    return vector(1, &x);

  vector_t* result = self;

  if (has_flag(self, FROZEN)) {
    if (result->length < MAXN) // space exists in the
      result = clone_object(self, 1);

    else {
      nodebuf_t buf;

      init_nodebuf(&buf, self);
      int h = push_tail(&buf);

      if (h < (int)buf.maxh)
        clone_nb_levels(&buf, h+1, buf.maxh);

      
    }
  } else {
    
  }

  result->tail[result->length++] = x;
  result->arity++;
  return result;
}

// dict -----------------------------------------------------------------------
typedef struct {
  stencil_t* levels[MAXL];
  uint8      offsets[MAXL];
} stbuf_t;

typedef struct {
  dict_t*    source;
  pair_t*    next;
  uint64     count;
  stbuf_t    buffer;
} diter_t;

static pair_t* init_diter(diter_t* it, dict_t* srcdict);
static pair_t* next_ditem(diter_t* it);
static bool    has_next_ditem(diter_t* it);

#undef SHIFT
#undef MAXN
#undef MAXS
#undef MINN
#undef MASK
#undef MAXH
#undef MAXL
