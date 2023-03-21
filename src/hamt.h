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
// array node -----------------------------------------------------------------
#define is_arr_node(x)   ISA(x, ArrNodeType)
#define as_arr_node(x)   ASP(x, arr_node_t)

arr_node_t* arr_node(usize height, usize len, void* dat);

// map node -------------------------------------------------------------------
#define is_map_node(x)   ISA(x, MapNodeType)
#define as_map_node(x)   ASP(x, map_node_t)

map_node_t* map_node(usize height, usize bitmap, void* dat);

// map leaf -------------------------------------------------------------------
#define is_map_leaf(x)   ISA(x, MapLeafType)
#define as_map_leaf(x)   ASP(x, map_leaf_t)

map_leaf_t* map_leaf(value_t key, value_t val);

// map leaves -----------------------------------------------------------------
#define is_map_leaves(x) ISA(x, MapLeavesType)
#define as_map_leaves(x) ASP(x, map_leaves_t)

map_leaves_t* map_leaves(value_t key, value_t val, map_leaves_t* prev);

// set node -------------------------------------------------------------------
#define is_set_node(x)   ISA(x, SetNodeType)
#define as_set_node(x)   ASP(x, set_node_t)

set_node_t* set_node(usize height, usize bitmap, void* dat);

// set leaf -------------------------------------------------------------------
#define is_set_leaf(x)   ISA(x, SetLeafType)
#define as_set_leaf(x)   ASP(x, set_leaf_t)

set_leaf_t* set_leaf(value_t val);

// set leaves -----------------------------------------------------------------
#define is_set_leaves(x) ISA(x, SetLeavesType)
#define as_set_leaves(x) ASP(x, set_leaves_t)

set_leaves_t* set_leaves(value_t val, set_leaves_t* prev);

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
