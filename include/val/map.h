#ifndef rl_map_h
#define rl_map_h

#include "val/val.h"

// Node types for the HAMT
typedef enum {
  MAP_LEAF,      // Single key-value pair
  MAP_BRANCH,    // Internal node with children
  MAP_COLLISION  // Multiple key-value pairs with same hash
} MapNodeKind;

// A HAMT node - either leaf, branch, or collision node
struct MapNode {
  HEAD;
  MapNodeKind kind;
  hash_t hash;  // hash of key (for leaf) or common hash prefix (for collision)
  union {
    struct { Expr key; Expr val; } leaf;   // MAP_LEAF
    BitVec children;                        // MAP_BRANCH
    Exprs pairs;                            // MAP_COLLISION (alternating key/val)
  };
};

// Top-level map object
struct Map {
  HEAD;
  MapNode* root;
  int count;  // total key-value pairs
};

// map API
Map* mk_map(RlState* rls);
Map* mk_map_s(RlState* rls);
bool map_get(Map* m, Expr key, Expr* val);
Map* map_assoc(RlState* rls, Map* m, Expr key, Expr val);
Map* map_dissoc(RlState* rls, Map* m, Expr key);
bool map_contains(Map* m, Expr key);
List* map_keys(RlState* rls, Map* m);
List* map_vals(RlState* rls, Map* m);

// convenience macros and accessors
#define as_map(x)        ((Map*)as_obj(x))
#define as_map_s(rls, x) ((Map*)as_obj_s(rls, &MapType, x))

#define as_map_node(x)        ((MapNode*)as_obj(x))
#define as_map_node_s(rls, x) ((MapNode*)as_obj_s(rls, &MapNodeType, x))

static inline int map_count(Map* m) {
  return m ? m->count : 0;
}

#define is_map(x) has_type(x, &MapType)

#endif
