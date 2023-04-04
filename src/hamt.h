#ifndef hamt_h
#define hamt_h

#include "object.h"

/* core user collection types and their internal dependencies */
// C types --------------------------------------------------------------------
struct vector_t {
  HEADER;
  uint64  arity;
  uint32  length;
  uint32  capacity;
  node_t* root;
  value_t tail[];
};

struct dict_t {
  HEADER;
  uint64     arity;
  stencil_t* root;
};

struct stencil_t {
  HEADER;
  uint64    bitmap;
  uint32    shift;
  uint16    length;
  uint16    capacity;

  object_t* children[];
};

struct node_t {
  HEADER;
  uint32 shift;
  uint16 length;
  uint16 capacity;

  union {
    value_t leaf[0];
    node_t* node[0];
  };
};

// globals --------------------------------------------------------------------
extern data_type_t VectorType, DictType, StencilType, NodeType;

extern vector_t EmptyVector;
extern dict_t   EmptyDict;

// API ------------------------------------------------------------------------
// internals ------------------------------------------------------------------
#define   is_node(x) ISA(x, NodeType)
#define   as_node(x) ASP(x, node_t)

node_t*   node(usize n, uint shift, void* data);

#define   is_stencil(x) ISA(x, StencilType)
#define   as_stencil(x) ASP(x, stencil_t)

// vector ---------------------------------------------------------------------
#define   is_vector(x) ISA(x, VectorType)
#define   as_vector(x) ASP(x, vector_t)

vector_t* vector(usize n, value_t* vs);
value_t   vector_get(vector_t* self, usize n);
vector_t* vector_set(vector_t* self, usize n, value_t x);
vector_t* vector_add(vector_t* self, value_t x);
vector_t* vector_del(vector_t* self);

// dict ----------------------------------------------------------------------
#define is_dict(x) ISA(x, DictType)
#define as_dict(x) ASP(x, dict_t)

dict_t* dict(usize n, value_t* kvs);
value_t dict_get(dict_t* self, value_t k);
dict_t* dict_set(dict_t* self, value_t k, value_t v);
dict_t* dict_del(dict_t* self, value_t k, value_t v);

#endif
