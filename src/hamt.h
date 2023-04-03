#ifndef hamt_h
#define hamt_h

#include "value.h"

/* core user collection types and their internal dependencies */
// C types --------------------------------------------------------------------
typedef struct {
  HEADER;

  union {
    uint64 arity;
    uint64 bitmap;
  };

  uint32   shift;
  uint8    length;
  uint8    capacity;
  uint8    depth;
  uint8    hasNul;
  value_t* tail;       // always allocated inline
} hamt_t;

typedef struct {
  hamt_t hamt;
  tuple_t* root;
} hamt_root_t;

struct vector_t {
  hamt_root_t root;
};

struct dict_t {
  hamt_root_t root;
  stencil_t*  map;
  value_t     nulval;
};

struct stencil_t {
  hamt_t  hamt;
};

// globals --------------------------------------------------------------------
extern data_type_t VectorType, DictType, StencilType;

extern vector_t EmptyVector;
extern dict_t   EmptyDict;

// API ------------------------------------------------------------------------
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
