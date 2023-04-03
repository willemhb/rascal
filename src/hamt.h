#ifndef hamt_h
#define hamt_h

#include "value.h"

/* core user collection types and their internal dependencies */
// C types --------------------------------------------------------------------

struct vecnode_t {
  HEADER;
  usize     length;
  object_t* children[64];
};

struct vecleaf_t {
  HEADER;
  value_t values[64];
};

struct mapnode_t {
  HEADER;
  uint64     bitmap;
  uint32     offset;
  uint16     count;
  uint16     capacity;
  object_t** children;
};

struct mapleaf_t {
  HEADER;
  mapleaf_t* next;
  value_t    key;
  value_t    val;
};

struct vector_t {
  HEADER;
  usize      arity;
  uint       offset;
  uint       length;
  vecnode_t* root;
  value_t    tail[64];
};

struct dict_t {
  HEADER;
  usize      len;
  mapnode_t* root;
};

struct set_t {
  HEADER;
  usize       len;
  mapnode_t*  root;
};

// globals --------------------------------------------------------------------
extern data_type_t VectorType, SetType, DictType,
  VecNodeType, VecLeafType, MapNodeType, MapLeafType;

extern vector_t EmptyVector;
extern dict_t   EmptyDict;
extern set_t    EmptySet;

// API ------------------------------------------------------------------------
// vector ---------------------------------------------------------------------
#define   is_vector(x) ISA(x, VectorType)
#define   as_vector(x) ASP(x, vector_t)

vector_t* vector(usize n, value_t* vs);
value_t   vector_get(vector_t* self, usize n);
vector_t* vector_set(vector_t* self, usize n, value_t x);
vector_t* vector_add(vector_t* self, value_t x);
vector_t* vector_del(vector_t* self, value_t x);

// dict ----------------------------------------------------------------------
#define is_dict(x) ISA(x, DictType)
#define as_dict(x) ASP(x, dict_t)

dict_t* dict(usize n, value_t* kvs);
value_t dict_get(dict_t* self, value_t k);
dict_t* dict_set(dict_t* self, value_t k, value_t v);
dict_t* dict_del(dict_t* self, value_t k, value_t v);

// set ------------------------------------------------------------------------
#define is_set(x) ISA(x, SetType)
#define as_set(x) ASP(x, set_t)

set_t* set(usize n, value_t* ks);
bool   set_has(set_t* self, value_t k);
set_t* set_add(set_t* self, value_t k);
set_t* set_del(set_t* self, value_t k);

#endif
