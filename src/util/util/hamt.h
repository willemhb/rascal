#ifndef rl_util_hamt_h
#define rl_util_hamt_h

#include "common.h"

typedef enum hamt_node_type_t hamt_node_type_t;
typedef enum hamt_table_type_t hamt_table_type_t;

typedef struct hamt_header_t hamt_header_t;
typedef struct hamt_node_t hamt_node_t;
typedef struct hamt_leaf_t hamt_leaf_t;
typedef struct hamt_collision_t hamt_collision_t;

typedef hamt_header_t *hamt_t;

enum hamt_node_type_t {
  hamt_leaf=-1,
  hamt_node_0, hamt_node_1, hamt_node_2,
  hamt_node_3, hamt_node_4, hamt_node_5,
  hamt_node_6, hamt_node_7, hamt_node_8
};

struct hamt_header_t {
  hamt_node_type_t node_type;
};

struct hamt_node_t {
  hamt_header_t header;
  ushort size; /* allocated size in bytes */
  uchar  length; /* count of used space (max 64) */
  uchar  capacity; /* count of available space (max 64) */
  hamt_node_t *cache;
  uchar space[0];
};

struct hamt_leaf_t {
  hamt_header_t header;
  ulong hash;
  hamt_leaf_t *next; // for resolving collisions
};

#endif
