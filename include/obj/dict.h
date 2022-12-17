#ifndef rl_obj_table_h
#define rl_obj_table_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef struct dict_node_t {
  int depth;
  ushort size;
  uchar length;
  uchar capacity;
  struct dict_node_t *cache;

  union {
    struct dict_hamt_t *children[0];
  };
} dict_node_t;

typedef struct dict_leaf_t {
  struct dict_leaf_t *next; /* in the event of a hash collision (unlikely) */
  ulong hash;
  rl_value_t key;
  rl_value_t bind;
} dict_leaf_t;

struct dict_t {
  RL_OBJ_HEADER;
  size_t count;
  dict_node_t *table;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_table_init( void );
void rl_obj_table_mark( void );
void rl_obj_table_cleanup( void );

/* convenience */

#endif
