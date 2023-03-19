#ifndef hamt_h
#define hamt_h

#include "value.h"

/* core user collection types and their internal dependencies */

// C types --------------------------------------------------------------------
struct arr_node_t {
  HEADER;
  uint32 len, cap;
  object_t** children;
};

struct arr_leaf_t {
  HEADER;
  uint32 len, cap;
  value_t* values;
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
  value_t key, val;
  map_leaves_t* next;
};

struct vector_t {
  HEADER;
  usize       len;
  arr_node_t* root;
};

struct dict_t {
  HEADER;
  usize       len;
  map_node_t* root;
};

struct set_t {
  HEADER;
  usize       len;
  map_node_t* root;
};

// API ------------------------------------------------------------------------

#endif
