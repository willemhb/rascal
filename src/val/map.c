#include "val/map.h"
#include "val/list.h"
#include "val/port.h"
#include "val/primitive.h"
#include "util/util.h"
#include "vm.h"
#include "lang.h"

// HAMT configuration
#define HASH_BITS 6
#define HASH_MASK 0x3F
#define MAX_DEPTH 8         // 48 bits / 6 bits per level

/*
// traverse helper
typedef struct {
  MapNode* node;
  int offset;
  MapNodeKind kind;
} NodeIter;

typedef struct {
  Map* root;
  int count;
  Expr key, val;
  NodeIter stack[MAX_DEPTH], *top;
} MapIter;
*/

// forward declarations
void print_map(Port* ios, Expr x);
void print_map_node(Port* ios, Expr x);
hash_t hash_map(Expr x);
bool egal_maps(Expr x, Expr y);
void trace_map(RlState* rls, void* ptr);
void trace_map_node(RlState* rls, void* ptr);
void free_map_node(RlState* rls, void* ptr);

/*
static void add_to_collision_node(RlState* rls, MapNode* coll, Expr key, Expr val);
static void map_iter_init(MapIter* iter, Map* map);
static bool map_iter_next(MapIter* iter);
static void map_iter_advance(MapIter* iter);
static void node_iter_init(NodeIter* iter, MapNode* map);
static MapNode* node_iter_next_child(NodeIter* iter);
*/

static MapNode* find_node(Map* map, Expr key);
static void map_insert(RlState* rls, Map* map, Expr key, Expr val);
static void map_delete(RlState* rls, StackRef mbuf, Expr key);

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
  n->shift = 0;
  n->leaf.key = key;
  n->leaf.val = val;
  return n;
}

static MapNode* mk_leaf_node_s(RlState* rls, hash_t hash, Expr key, Expr val) {
  MapNode* out = mk_leaf_node(rls, hash, key, val);
  stack_push(rls, tag_obj(out));
  return out;
}

static MapNode* mk_branch_node(RlState* rls, int shift) {
  MapNode* n = mk_obj(rls, &MapNodeType, 0);
  n->kind = MAP_BRANCH;
  n->hash = 0;
  n->shift = shift;
  init_bit_vec(rls, &n->children);
  return n;
}

static MapNode* mk_branch_node_s(RlState* rls, int shift) {
  MapNode* out = mk_branch_node(rls, shift);
  stack_push(rls, tag_obj(out));
  return out;
}

// clone a node for path copying
static MapNode* clone_node(RlState* rls, MapNode* src) {
  MapNode* dst = clone_obj(rls, src);

  if ( src->kind == MAP_BRANCH )
    bit_vec_clone(rls, &src->children, &dst->children);

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

/*
static void map_iter_init(MapIter* iter, Map* map) {
  iter->root = map;
  iter->count = 0;
  iter->val = NONE;
  iter->key = NONE;
  iter->top = iter->stack;

  if ( map->count > 0 ) {
    MapNode* node = map->root;

    while ( node->kind == MAP_BRANCH ) {
      node_iter_init(iter->top++, node);
      node = node_iter_next_child(iter->top);
    }

    node_iter_init(iter->top++, node);
  }
}

static bool map_iter_next(MapIter* iter) {
  bool out;

  if ( iter->root->count == iter->count ) {
    out = false;
  } else {
    out = true;
    map_iter_advance(iter);
    iter->count++;
  }

  return out;
}

static void map_iter_advance(MapIter* iter) {
  bool exhausted;

  NodeIter* node_iter = iter->top-1;
  
  if ( node_iter->kind == MAP_LEAF ) {
    iter->key = node_iter->node->leaf.key;
    iter->val = node_iter->node->leaf.val;
    exhausted = true;
    
  } else {
    assert(node_iter->kind == MAP_COLLISION);
    
    iter->key = node_iter->node->pairs.data[node_iter->offset++];
    iter->val = node_iter->node->pairs.data[node_iter->offset++];
    exhausted = node_iter->offset == node_iter->node->pairs.count;
  }

  if ( exhausted ) { // find next leaf or collision node
    iter->top--;

    MapNode* next_leaf = NULL, *next_child;

    while ( next_leaf == NULL ) {
      next_child = node_iter_next_child(iter->top-1);

      if ( next_child == NULL ) {
        if ( --iter->top == iter->stack ) // end of everything
          break;
      } else if ( next_child->kind == MAP_BRANCH ) {
        node_iter_init(iter->top++, next_child);
      } else {
        next_leaf = next_child;
      }
    }

    if ( next_leaf != NULL )
      node_iter_init(iter->top++, next_leaf);
  }
}

static void node_iter_init(NodeIter* iter, MapNode* node) {
  iter->node   = node;
  iter->kind   = node->kind;
  iter->offset = node->kind == MAP_BRANCH ? -1 : 0; // simplefies node_iter_next_child
}

static MapNode* node_iter_next_child(NodeIter* iter) {
  assert(iter->node->kind == MAP_BRANCH);
  MapNode* node = iter->node, *out = NULL;
  void** children = iter->node->children.data;

  while ( out == NULL && ++iter->offset < node->children.maxc )
    out = children[iter->offset];

  return out;
}
*/

// map API
static Map* new_map(RlState* rls) {
  Map* m = mk_obj(rls, &MapType, 0);
  m->root = NULL;
  m->count = 0;
  m->transient = true;

  return m;
}

static Map* new_map_s(RlState* rls) {
  Map* m = new_map(rls);
  stack_push(rls, tag_obj(m));
  return m;
}

Map* mk_map(RlState* rls, int n) {
  Map* map = new_map_s(rls);

  if ( n > 0 ) {
    StackRef top = rls->s_top;
    StackRef base = top-n;
 
    map->root = mk_branch_node(rls, 0);

    for ( int i=0; i<n; i+= 2 ) {
      Expr key = base[i];
      Expr val = base[i+1];
      map_insert(rls, map, key, val);
    }
  }

  // mark as immutable
  map->transient = false;

  return map;
}

Map* mk_map_s(RlState* rls, int n) {
  Map* out = mk_map(rls, n);
  stack_push(rls, tag_obj(out));
  return out;
}

// lookup implementation
static bool branch_node_has(MapNode* branch, int index) {
  return bit_vec_has(&branch->children, index);
}

static MapNode* branch_node_get(MapNode* branch, int index) {
  return bit_vec_get(&branch->children, index);
}

static void branch_node_set(RlState* rls, MapNode* branch, int index, MapNode* child) {
  bit_vec_set(rls, &branch->children, index, child);
}

static void branch_node_remove(RlState* rls, MapNode* branch, int index) {
  bit_vec_remove(rls, &branch->children, index);
}

static MapNode* find_node(Map* map, Expr key) {
  MapNode* node = map->root, *out = NULL;

  hash_t hash = hash_expr(key);

  for (;;) {
    if ( node == NULL )
      break;

    if ( is_leaf(node) ) {
      if ( egal_exprs(key, leaf_key(node)) )
        out = node;

      assert(out != NULL || node->hash != hash);

      break;
    }

    int shift = node->shift;
    int index = hash_index(hash, shift);

    if ( branch_node_has(node, index) )
      node = branch_node_get(node, index);

    else
      break;
  }

  return out;
}

Expr map_get(Map* map, Expr key) {
  MapNode* node = find_node(map, key);

  if ( node == NULL )
    return NONE;

  return leaf_val(node);
}

bool map_contains(Map* map, Expr key) {
  return find_node(map, key) != NULL;
}

// recursive insert helper
static void map_insert(RlState* rls, Map* map, Expr key, Expr val) {
  MapNode* node, *childnode, *newnode, *leafnode;
  int shift, newshift, index, childindex;
  bool transient;
  StackRef top;
  hash_t hash, childhash;
  top = rls->s_top;
  node = map->root;
  hash = hash_expr(key);
  transient = map->transient;

  for (;;) {
    if ( transient )
      node = clone_node_s(rls, node);

    shift = node->shift;
    index = hash_index(hash, shift);

    if ( !branch_node_has(node, index) ) { // simplest case
      newnode = mk_leaf_node(rls, hash, key, val);
      branch_node_set(rls, node, index, newnode);
      map->count++;
      break;
    } else {
      childnode = branch_node_get(node, index);

      if ( is_branch(childnode) )
        node = childnode;

      else if ( egal_exprs(key, leaf_key(childnode)) ) {
        if ( transient )
          childnode = clone_node_s(rls, node);

        leaf_val(childnode) = val;
        break;
      } else {
        // not dealing with collisions yet
        childhash = childnode->hash;
        assert(childhash != hash);
        leafnode = childnode;
        childindex = index;
        stack_push(rls, tag_obj(leafnode));

        // find the level where they split and insert there
        while ( childindex == index ) {
          newshift = shift + HASH_BITS;
          newnode = mk_branch_node_s(rls, newshift);
          branch_node_set(rls, node, index, newnode);
          node = newnode;
          index = hash_index(hash, shift);
          childindex = hash_index(childhash, shift);
        }

        newnode = mk_leaf_node(rls, hash, key, val);
        branch_node_set(rls, node, index, newnode);
        branch_node_set(rls, node, childindex, childnode);

        map->count++;
        break;
      }
    }
  }

  rls->s_top = top;
}

static void map_delete(RlState* rls, StackRef mbuf, Expr key) {
  Map* map;
  MapNode* node, *leafnode;
  int shift, index;
  bool transient;
  StackRef top, last;
  hash_t hash;
  map = as_map(*mbuf);
  top = rls->s_top;
  last = stack_push(rls, NUL);
  node = map->root;
  hash = hash_expr(key);
  transient = map->transient;
  hash = hash_expr(key);
  node = map->root;
  leafnode = NULL;

  while ( node != NULL && leafnode == NULL ) {
    if ( is_leaf(node) ) {
      if ( egal_exprs(key, leaf_key(node)) )
        leafnode = node;

      assert(leafnode != NULL || hash != node->hash);
    } else {
      shift = node->shift;
      index = hash_index(hash, shift);

      if ( !branch_node_has(node, index) )
        node = NULL;

      else {
        stack_push(rls, tag_obj(node));
        stack_push(rls, tag_fix(index));
        node = branch_node_get(node, index);
      }
    }
  }

  if ( leafnode != NULL ) { // key was found and removed,
    if ( transient ) {
      map = clone_obj(rls, map);
      *mbuf = tag_obj(map);
    }

    if ( map->count == 0 )
      map->root = NULL;

    else {
      StackRef bottom = last + 1;

      while ( rls->s_top > bottom ) {
        index = as_fix(stack_pop(rls));
        node = as_map_node(stack_pop(rls));

        // the complex case. We need to remove singleton branches (if any exist)
        // and, if the last non-singleton branch 
        if ( *last == NUL ) {  
          if ( branch_count(node) == 1 ) // remove singleton branches
            continue;

          // if the first non-singleton branch is two leaf nodes, delete the node
          // and move the other leaf to the parent
          if ( branch_count(node) == 2 ) {
            int cindex = bitmap_to_index(node->children.bitmap, index);
            int oindex = cindex == 1 ? 0 : 1;
            MapNode* ochild = branch_children(node)[oindex];

            if ( is_leaf(ochild) ) {
              *last = tag_obj(ochild);
              continue;
            }
          }

          if ( transient )
            node = clone_node(rls, node);

          branch_node_remove(rls, node, index);
          *last = tag_obj(node);
        } else {
          // all other cases are simple
          if ( transient )
            node = clone_node(rls, node);

          branch_node_set(rls, node, index, as_map_node(*last));
          *last = tag_obj(node);
        }
      }

      map->root = as_map_node(*last);
    }
  }

  rls->s_top = top;
}


Map* map_assoc(RlState* rls, Map* map, Expr key, Expr val) {
  StackRef top = rls->s_top;

  if ( !map->transient )
    map = clone_obj_s(rls, map);

  if ( map->root == NULL )
    map->root = mk_branch_node(rls, 0);

  map_insert(rls, map, key, val);
  rls->s_top = top;
  return map;
}

// recursive dissoc helper
Map* map_dissoc(RlState* rls, Map* map, Expr key) {
  if ( map->count > 0 ) {
    Expr* mbuf = stack_push(rls, tag_obj(map));
    map_delete(rls, mbuf, key);
    map = as_map(stack_pop(rls));
  }

    return map;
}

// collect keys/vals helpers
static void collect_keys(RlState* rls, MapNode* node) {
  if ( node == NULL ) return;

  switch ( node->kind ) {
    case MAP_LEAF:
      stack_push(rls, node->leaf.key);
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
static void print_map_nodes(Port* ios, MapNode* node, int* count, int max_count) {
  if ( is_leaf(node) ) {
    print_expr(ios, leaf_key(node));
    pprintf(ios, " ");
    print_expr(ios, leaf_val(node));

    if ( ++(*count) < max_count )
      pprintf(ios, " ");

  } else {
    for ( int i=0; i < branch_count(node); i++ ) {
      print_map_nodes(ios, branch_children(node)[i], count, max_count);
    }
  }
}

static void hash_map_nodes(MapNode* node, hash_t* seed) {
  if ( is_leaf(node) ) {
    *seed = mix_hashes_48(*seed, hash_expr(leaf_key(node)));
    *seed = mix_hashes_48(*seed, hash_expr(leaf_key(node)));
  } else {
    for ( int i=0; i < branch_count(node); i++ )
      hash_map_nodes(branch_children(node)[i], seed);
  }
}

static bool egal_map_nodes(MapNode* nx, Map* my) {
  bool out = true;
  if ( is_leaf(nx) ) {
    MapNode* ny = find_node(my, leaf_key(nx));

    if ( ny == NULL )
      out = false;

    else
      out = egal_exprs(leaf_val(nx), leaf_val(ny));
  } else {
    for ( int i=0; out && i < branch_count(nx); i++ )
      out = egal_map_nodes(branch_children(nx)[i], my);
  }

  return out;
}

void print_map(Port* ios, Expr x) {
  Map* map = as_map(x);
  int count = 0, max_count = map->count;
  pprintf(ios, "{");
  print_map_nodes(ios, map->root, &count, max_count);
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
  Map* map = as_map(x);
  hash_t hash = hash_word_48(map->count+1); // guards against weird shit with empty collections
  hash_map_nodes(map->root, &hash);
  return mix_hashes_48(MapType.hashcode, hash);
}

bool egal_maps(Expr x, Expr y) {
  Map* mx = as_map(x);
  Map* my = as_map(y);

  if ( mx->count != my->count )
    return false;

  if ( mx->count == 0 )
    return true;

  // Check that all keys in mx exist in my with same values
  // This is O (n * log64 n) where n is the size of both maps
  return egal_map_nodes(mx->root, my);
}

// GC functions
void trace_map(RlState* rls, void* ptr) {
  Map* m = ptr;
  mark_obj(rls, m->root);
}

void trace_map_node(RlState* rls, void* ptr) {
  MapNode* n = ptr;

  if ( is_leaf(n) ) {
      mark_expr(rls, n->leaf.key);
      mark_expr(rls, n->leaf.val);
  } else {
    for ( int i = 0; i < n->children.count; i++ )
      mark_obj(rls, n->children.data[i]);
  }
}

void free_map_node(RlState* rls, void* ptr) {
  MapNode* n = ptr;

  if ( n->kind == MAP_BRANCH )
    free_bit_vec(rls, &n->children);
}
