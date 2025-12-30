#ifndef rl_map_h
#define rl_map_h

#include "val/val.h"

// Node types for the HAMT
typedef enum {
  MAP_LEAF,      // Single key-value pair
  MAP_BRANCH,    // Internal node with children
} MapNodeKind;

// A HAMT node - either leaf or a branch
// this implementation doesn't handle collisions yet
// since these are astronomically unlikely to happen with 48 bit hashing
// I am choosing to assume this will never happen until an actual public
// release is in the offing.
struct MapNode {
  HEAD;
  MapNodeKind kind;
  int shift;
  hash_t hash;

  union {
    struct {
      Expr     key;
      Expr     val;
      MapNode* next; // for collisions -- currently not implemented, but the
                     // extra space costs nothing so this seems like a reasonable implementation
    } leaf;          // MAP_LEAF
    BitVec children; // MAP_BRANCH
  };
};

// Top-level map object
struct Map {
  HEAD;
  MapNode* root;
  int  count;     // total key-value pairs
  bool transient; // indicates whether updates can be in place (set temporarily during construction)
};

// map API
Map* mk_map(RlState* rls, int n);
Map* mk_map_s(RlState* rls, int n);
Expr  map_get(Map* m, Expr key);
Map* map_assoc(RlState* rls, Map* m, Expr key, Expr val);
Map* map_dissoc(RlState* rls, Map* m, Expr key, MapNode** rmv);
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

#define is_map(x)          has_type(x, &MapType)
#define is_leaf(n)         ((n)->kind == MAP_LEAF)
#define is_branch(n)       ((n)->kind == MAP_BRANCH)
#define node_hash(n)       ((n)->hash)
#define branch_children(n) ((n)->children.data)
#define branch_maxc(n)     ((n)->children.maxc)
#define branch_count(n)    ((n)->children.count)
#define leaf_key(n)        ((n)->leaf.key)
#define leaf_val(n)        ((n)->leaf.val)

#endif
