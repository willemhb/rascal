#include "val/map.h"
#include "val/list.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// HAMT configuration
#define HASH_BITS 6
#define HASH_MASK 0x3F
#define MAX_DEPTH 11  // 64 bits / 6 bits per level

// forward declarations
void print_map(Port* ios, Expr x);
void print_map_node(Port* ios, Expr x);
hash_t hash_map(Expr x);
bool egal_maps(Expr x, Expr y);
void trace_map(RlState* rls, void* ptr);
void trace_map_node(RlState* rls, void* ptr);
void free_map_node(RlState* rls, void* ptr);

// Type objects
Type MapNodeType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_MAP_NODE,
  .obsize   = sizeof(MapNode),
  .print_fn = print_map_node,
  .trace_fn = trace_map_node,
  .free_fn  = free_map_node,
};

Type MapType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_MAP,
  .obsize   = sizeof(Map),
  .print_fn = print_map,
  .hash_fn  = hash_map,
  .egal_fn  = egal_maps,
  .trace_fn = trace_map,
};

// internal node constructors
static MapNode* mk_leaf_node(RlState* rls, hash_t hash, Expr key, Expr val) {
  MapNode* n = mk_obj(rls, &MapNodeType, 0);
  n->kind = MAP_LEAF;
  n->hash = hash;
  n->leaf.key = key;
  n->leaf.val = val;
  return n;
}

static MapNode* mk_leaf_node_s(RlState* rls, hash_t hash, Expr key, Expr val) {
  MapNode* out = mk_leaf_node(rls, hash, key, val);
  stack_push(rls, tag_obj(out));
  return out;
}

static MapNode* mk_branch_node(RlState* rls) {
  MapNode* n = mk_obj(rls, &MapNodeType, 0);
  n->kind = MAP_BRANCH;
  n->hash = 0;
  init_bit_vec(rls, &n->children);
  return n;
}

static MapNode* mk_branch_node_s(RlState* rls) {
  MapNode* out = mk_branch_node(rls);
  stack_push(rls, tag_obj(out));
  return out;
}

static MapNode* mk_collision_node(RlState* rls, hash_t hash) {
  MapNode* n = mk_obj(rls, &MapNodeType, 0);
  n->kind = MAP_COLLISION;
  n->hash = hash;
  init_exprs(rls, &n->pairs);
  return n;
}

static MapNode* mk_collision_node_s(RlState* rls, hash_t hash) {
  MapNode* out = mk_collision_node(rls, hash);
  stack_push(rls, tag_obj(out));
  return out;
}

// clone a node for path copying
static MapNode* clone_node(RlState* rls, MapNode* src) {
  MapNode* dst = clone_obj(rls, src);

  if ( src->kind == MAP_BRANCH ) {
    bit_vec_clone(rls, &src->children, &dst->children);
  } else if ( src->kind == MAP_COLLISION ) {
    init_exprs(rls, &dst->pairs);
    exprs_write(rls, &dst->pairs, src->pairs.data, src->pairs.count);
  }

  return dst;
}

static MapNode* clone_node_s(RlState* rls, MapNode* src) {
  MapNode* out = clone_node(rls, src);
  stack_push(rls, tag_obj(out));
  return out;
}

// get index from hash at given depth (shift)
static inline int hash_index(hash_t hash, int shift) {
  return (hash >> shift) & HASH_MASK;
}

// collision node helpers
static int collision_find(MapNode* node, Expr key) {
  assert(node->kind == MAP_COLLISION);
  for ( int i = 0; i < node->pairs.count; i += 2 ) {
    if ( egal_exprs(node->pairs.data[i], key) )
      return i;
  }
  return -1;
}

// map API
Map* mk_map(RlState* rls) {
  Map* m = mk_obj(rls, &MapType, 0);
  m->root = NULL;
  m->count = 0;
  return m;
}

Map* mk_map_s(RlState* rls) {
  Map* out = mk_map(rls);
  stack_push(rls, tag_obj(out));
  return out;
}

// lookup implementation
bool map_get(Map* m, Expr key, Expr* val) {
  if ( m->root == NULL )
    return false;

  hash_t hash = hash_expr(key);
  MapNode* node = m->root;
  int shift = 0;

  while ( node != NULL ) {
    switch ( node->kind ) {
      case MAP_LEAF:
        if ( egal_exprs(node->leaf.key, key) ) {
          if ( val ) *val = node->leaf.val;
          return true;
        }
        return false;

      case MAP_COLLISION: {
        int idx = collision_find(node, key);
        if ( idx >= 0 ) {
          if ( val ) *val = node->pairs.data[idx + 1];
          return true;
        }
        return false;
      }

      case MAP_BRANCH: {
        int idx = hash_index(hash, shift);
        if ( !bit_vec_has(&node->children, idx) )
          return false;
        node = bit_vec_get(&node->children, idx);
        shift += HASH_BITS;
        break;
      }
    }
  }

  return false;
}

bool map_contains(Map* m, Expr key) {
  return map_get(m, key, NULL);
}

// recursive insert helper
static MapNode* node_assoc(RlState* rls, MapNode* node, hash_t hash, Expr key, Expr val, int shift, int* added) {
  if ( node == NULL ) {
    *added = 1;
    return mk_leaf_node(rls, hash, key, val);
  }

  switch ( node->kind ) {
    case MAP_LEAF: {
      if ( egal_exprs(node->leaf.key, key) ) {
        // update existing key
        *added = 0;
        return mk_leaf_node(rls, hash, key, val);
      }

      if ( node->hash == hash ) {
        // hash collision - create collision node
        *added = 1;
        StackRef top = rls->s_top;
        stack_push(rls, key);
        stack_push(rls, val);
        MapNode* coll = mk_collision_node(rls, hash);
        exprs_push(rls, &coll->pairs, node->leaf.key);
        exprs_push(rls, &coll->pairs, node->leaf.val);
        exprs_push(rls, &coll->pairs, key);
        exprs_push(rls, &coll->pairs, val);
        rls->s_top = top;
        return coll;
      }

      // different hash - create branch with both nodes
      *added = 1;
      StackRef top = rls->s_top;
      stack_push(rls, tag_obj(node));
      stack_push(rls, key);
      stack_push(rls, val);

      MapNode* branch = mk_branch_node(rls);
      int idx1 = hash_index(node->hash, shift);
      int idx2 = hash_index(hash, shift);

      if ( idx1 == idx2 ) {
        // same slot - recurse
        MapNode* child = node_assoc(rls, node, hash, key, val, shift + HASH_BITS, added);
        bit_vec_set(rls, &branch->children, idx1, child);
      } else {
        // different slots
        MapNode* leaf = mk_leaf_node(rls, hash, key, val);
        bit_vec_set(rls, &branch->children, idx1, node);
        bit_vec_set(rls, &branch->children, idx2, leaf);
      }

      rls->s_top = top;
      return branch;
    }

    case MAP_COLLISION: {
      int idx = collision_find(node, key);
      StackRef top = rls->s_top;
      stack_push(rls, key);
      stack_push(rls, val);

      if ( idx >= 0 ) {
        // update existing in collision
        *added = 0;
        MapNode* newnode = clone_node(rls, node);
        newnode->pairs.data[idx + 1] = val;
        rls->s_top = top;
        return newnode;
      }

      // add to collision
      *added = 1;
      MapNode* newnode = clone_node(rls, node);
      exprs_push(rls, &newnode->pairs, key);
      exprs_push(rls, &newnode->pairs, val);
      rls->s_top = top;
      return newnode;
    }

    case MAP_BRANCH: {
      int idx = hash_index(hash, shift);
      StackRef top = rls->s_top;
      stack_push(rls, tag_obj(node));
      stack_push(rls, key);
      stack_push(rls, val);

      MapNode* newnode = clone_node(rls, node);
      MapNode* child = bit_vec_get(&node->children, idx);
      MapNode* newchild = node_assoc(rls, child, hash, key, val, shift + HASH_BITS, added);

      if ( child == NULL ) {
        bit_vec_set(rls, &newnode->children, idx, newchild);
      } else {
        bit_vec_update(rls, &newnode->children, idx, newchild);
      }

      rls->s_top = top;
      return newnode;
    }
  }

  return NULL; // unreachable
}

Map* map_assoc(RlState* rls, Map* m, Expr key, Expr val) {
  StackRef top = rls->s_top;
  stack_push(rls, tag_obj(m));
  stack_push(rls, key);
  stack_push(rls, val);

  int added = 0;
  hash_t hash = hash_expr(key);
  MapNode* newroot = node_assoc(rls, m->root, hash, key, val, 0, &added);

  Map* newmap = mk_map(rls);
  newmap->root = newroot;
  newmap->count = m->count + added;

  rls->s_top = top;
  return newmap;
}

// recursive dissoc helper
static MapNode* node_dissoc(RlState* rls, MapNode* node, hash_t hash, Expr key, int shift, int* removed) {
  if ( node == NULL ) {
    *removed = 0;
    return NULL;
  }

  switch ( node->kind ) {
    case MAP_LEAF: {
      if ( egal_exprs(node->leaf.key, key) ) {
        *removed = 1;
        return NULL;
      }
      *removed = 0;
      return node;
    }

    case MAP_COLLISION: {
      int idx = collision_find(node, key);
      if ( idx < 0 ) {
        *removed = 0;
        return node;
      }

      *removed = 1;

      if ( node->pairs.count == 4 ) {
        // only 2 pairs, removing one leaves just one - convert to leaf
        int other = (idx == 0) ? 2 : 0;
        return mk_leaf_node(rls, node->hash, node->pairs.data[other], node->pairs.data[other + 1]);
      }

      // clone and remove pair
      MapNode* newnode = clone_node(rls, node);
      // shift remaining pairs down
      for ( int i = idx; i < newnode->pairs.count - 2; i++ ) {
        newnode->pairs.data[i] = newnode->pairs.data[i + 2];
      }
      newnode->pairs.count -= 2;
      return newnode;
    }

    case MAP_BRANCH: {
      int idx = hash_index(hash, shift);

      if ( !bit_vec_has(&node->children, idx) ) {
        *removed = 0;
        return node;
      }

      StackRef top = rls->s_top;
      stack_push(rls, tag_obj(node));

      MapNode* child = bit_vec_get(&node->children, idx);
      MapNode* newchild = node_dissoc(rls, child, hash, key, shift + HASH_BITS, removed);

      if ( *removed == 0 ) {
        rls->s_top = top;
        return node;
      }

      if ( newchild == NULL ) {
        // child removed entirely
        if ( node->children.count == 1 ) {
          // branch becomes empty
          rls->s_top = top;
          return NULL;
        }

        if ( node->children.count == 2 ) {
          // check if we can collapse to a single leaf/collision
          int other_idx = -1;
          for ( int i = 0; i < 64; i++ ) {
            if ( i != idx && bit_vec_has(&node->children, i) ) {
              other_idx = i;
              break;
            }
          }
          MapNode* other = bit_vec_get(&node->children, other_idx);
          if ( other->kind != MAP_BRANCH ) {
            // collapse
            rls->s_top = top;
            return other;
          }
        }

        // just remove the slot
        MapNode* newnode = clone_node(rls, node);
        bit_vec_remove(rls, &newnode->children, idx);
        rls->s_top = top;
        return newnode;
      }

      // update child
      MapNode* newnode = clone_node(rls, node);
      bit_vec_update(rls, &newnode->children, idx, newchild);
      rls->s_top = top;
      return newnode;
    }
  }

  return NULL; // unreachable
}

Map* map_dissoc(RlState* rls, Map* m, Expr key) {
  StackRef top = rls->s_top;
  stack_push(rls, tag_obj(m));
  stack_push(rls, key);

  int removed = 0;
  hash_t hash = hash_expr(key);
  MapNode* newroot = node_dissoc(rls, m->root, hash, key, 0, &removed);

  if ( removed == 0 ) {
    rls->s_top = top;
    return m;
  }

  Map* newmap = mk_map(rls);
  newmap->root = newroot;
  newmap->count = m->count - 1;

  rls->s_top = top;
  return newmap;
}

// collect keys/vals helpers
static void collect_keys(RlState* rls, MapNode* node) {
  if ( node == NULL ) return;

  switch ( node->kind ) {
    case MAP_LEAF:
      stack_push(rls, node->leaf.key);
      break;
    case MAP_COLLISION:
      for ( int i = 0; i < node->pairs.count; i += 2 )
        stack_push(rls, node->pairs.data[i]);
      break;
    case MAP_BRANCH:
      for ( int i = 0; i < node->children.count; i++ )
        collect_keys(rls, node->children.data[i]);
      break;
  }
}

static void collect_vals(RlState* rls, MapNode* node) {
  if ( node == NULL ) return;

  switch ( node->kind ) {
    case MAP_LEAF:
      stack_push(rls, node->leaf.val);
      break;
    case MAP_COLLISION:
      for ( int i = 0; i < node->pairs.count; i += 2 )
        stack_push(rls, node->pairs.data[i + 1]);
      break;
    case MAP_BRANCH:
      for ( int i = 0; i < node->children.count; i++ )
        collect_vals(rls, node->children.data[i]);
      break;
  }
}

List* map_keys(RlState* rls, Map* m) {
  StackRef top = rls->s_top;
  stack_push(rls, tag_obj(m));
  collect_keys(rls, m->root);
  int count = (rls->s_top - top) - 1;
  List* out = mk_list(rls, count);
  rls->s_top = top;
  return out;
}

List* map_vals(RlState* rls, Map* m) {
  StackRef top = rls->s_top;
  stack_push(rls, tag_obj(m));
  collect_vals(rls, m->root);
  int count = (rls->s_top - top) - 1;
  List* out = mk_list(rls, count);
  rls->s_top = top;
  return out;
}

// print functions
static void print_map_node_entries(Port* ios, MapNode* node, int* first) {
  if ( node == NULL ) return;

  switch ( node->kind ) {
    case MAP_LEAF:
      if ( !*first ) pprintf(ios, " ");
      *first = 0;
      print_expr(ios, node->leaf.key);
      pprintf(ios, " ");
      print_expr(ios, node->leaf.val);
      break;
    case MAP_COLLISION:
      for ( int i = 0; i < node->pairs.count; i += 2 ) {
        if ( !*first ) pprintf(ios, " ");
        *first = 0;
        print_expr(ios, node->pairs.data[i]);
        pprintf(ios, " ");
        print_expr(ios, node->pairs.data[i + 1]);
      }
      break;
    case MAP_BRANCH:
      for ( int i = 0; i < node->children.count; i++ )
        print_map_node_entries(ios, node->children.data[i], first);
      break;
  }
}

void print_map(Port* ios, Expr x) {
  Map* m = as_map(x);
  pprintf(ios, "{");
  int first = 1;
  print_map_node_entries(ios, m->root, &first);
  pprintf(ios, "}");
}

void print_map_node(Port* ios, Expr x) {
  MapNode* n = as_map_node(x);
  pprintf(ios, "<map-node:%s>",
    n->kind == MAP_LEAF ? "leaf" :
    n->kind == MAP_BRANCH ? "branch" : "collision");
}

// hash and equality
hash_t hash_map(Expr x) {
  Map* m = as_map(x);
  // Simple hash combining count and root pointer for now
  // A proper implementation would combine hashes of all entries
  return (hash_t)m->count ^ (hash_t)(uintptr_t)m->root;
}

bool egal_maps(Expr x, Expr y) {
  Map* mx = as_map(x);
  Map* my = as_map(y);

  if ( mx->count != my->count )
    return false;

  if ( mx->count == 0 )
    return true;

  // Check that all keys in mx exist in my with same values
  // This is O(n) where n is the number of entries
  // We'll iterate through mx and look up each key in my
  // (A recursive tree comparison would be more efficient but more complex)

  // For now, use a simple approach - this could be optimized
  return mx == my; // pointer equality for now, full comparison is complex
}

// GC functions
void trace_map(RlState* rls, void* ptr) {
  Map* m = ptr;
  mark_obj(rls, m->root);
}

void trace_map_node(RlState* rls, void* ptr) {
  MapNode* n = ptr;

  switch ( n->kind ) {
    case MAP_LEAF:
      mark_expr(rls, n->leaf.key);
      mark_expr(rls, n->leaf.val);
      break;
    case MAP_COLLISION:
      trace_exprs(rls, &n->pairs);
      break;
    case MAP_BRANCH:
      for ( int i = 0; i < n->children.count; i++ )
        mark_obj(rls, n->children.data[i]);
      break;
  }
}

void free_map_node(RlState* rls, void* ptr) {
  MapNode* n = ptr;

  if ( n->kind == MAP_BRANCH )
    free_bit_vec(rls, &n->children);
  else if ( n->kind == MAP_COLLISION )
    free_exprs(rls, &n->pairs);
}
