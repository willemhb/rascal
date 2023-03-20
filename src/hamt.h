#ifndef hamt_h
#define hamt_h

#include "value.h"

/* core user collection types and their internal dependencies */
// C types --------------------------------------------------------------------
struct arr_node_t {
  HEADER;
  usize len;

  union {
    arr_node_t** children;
    value_t*     values;
    void*        data;
  };
};

struct map_node_t {
  HEADER;
  usize bitmap;
  object_t **children;
};

struct map_leaf_t {
  HEADER;
  value_t key, val;
};

struct map_leaves_t {
  HEADER;
  value_t       key;
  value_t       val;
  map_leaves_t* next;
};

struct set_node_t {
  HEADER;
  usize      bitmap;
  object_t** children;
};

struct set_leaf_t {
  HEADER;
  value_t key;
};

struct set_leaves_t {
  HEADER;
  value_t       key;
  set_leaves_t* next;
};

struct vector_t {
  HEADER;
  usize       len;
  arr_node_t *root;
};

struct dict_t {
  HEADER;
  usize       len;
  map_node_t* root;
};

struct set_t {
  HEADER;
  usize       len;
  set_node_t* root;
};

// globals --------------------------------------------------------------------
extern data_type_t VectorType, SetType, DictType,
  ArrNodeType, ArrLeafType,
  MapNodeType, MapLeafType, MapLeavesType,
  SetNodeType, SetLeafType, SetLeavesType;

extern vector_t EmptyVector;
extern dict_t   EmptyDict;
extern set_t    EmptySet;

// API ------------------------------------------------------------------------
// vector ---------------------------------------------------------------------
vector_t* vector(usize n, value_t* vs);
value_t   vector_get(vector_t* self, usize n);
vector_t* vector_set(vector_t* self, usize n, value_t x);
vector_t* vector_add(vector_t* self, value_t x);
vector_t* vector_del(vector_t* self, value_t x);

// dict -----------------------------------------------------------------------
dict_t* dict(usize n, value_t* kvs);
value_t dict_get(dict_t* self, value_t k);
dict_t* dict_set(dict_t* self, value_t k, value_t v);
dict_t* dict_del(dict_t* self, value_t k, value_t v);

// set ------------------------------------------------------------------------
set_t* set(usize n, value_t* ks);
bool   set_has(set_t* self, value_t k);
set_t* set_add(set_t* self, value_t k);
set_t* set_del(set_t* self, value_t k);

#endif
