#include <string.h>

#include "lang/dispatch.h"
#include "util/util.h"
#include "vm.h"
#include "val.h"

// C types --------------------------------------------------------------------
typedef enum {
  NO_MATCH=-1,
  EXACT_MATCH=0,
  INEXACT_MATCH=1
} DispatchResult;

// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
Method* get_cached_method(RlState* rls, MethodTable* mt, Tuple* sig);
void cache_method(RlState* rls, MethodTable* mt, Tuple* sig, Method* method);
int order_methods(Method* mx, Method* my);
void save_candidate(Method* method, Method** candidate, Method** collision);
DispatchResult mtnode_lookup(RlState* rls, MTNode* node, Tuple* sig, int argc, DispatchResult result, Method** candidate, Method** collision);
bool mtnode_add(RlState* rls, MTNode* node, Method* method);

// Implementations ------------------------------------------------------------
// Internal
Method* get_cached_method(RlState* rls, MethodTable* mt, Tuple* sig) {
  Method* out = NULL;
  table_get(rls, &mt->cache, sig, &out);
  return out;
}

void cache_method(RlState* rls, MethodTable* mt, Tuple* sig, Method* method) {
  table_set(rls, &mt->cache, sig, method, NULL);
}

// External
void fun_add_method(RlState* rls, Fun* fun, Method* m) {
  assert(fun->generic || fun->method == NULL);

  int count = fun->mcount;

  if ( count == 0 )
    fun->method = m;

  else {
    if ( count == 1 ) {
      MethodTable* mt = mk_mtable_s(rls, fun);
      mtable_add(rls, mt, fun->method);
      fun->methods = mt;
      stack_pop(rls);
    }

    mtable_add(rls, fun->methods, m);
  }

  fun->mcount++;
}

void fun_add_method_s(RlState* rls, Fun* fun, Method* m) {
  stack_preserve(rls, 2, tag_obj(fun), tag_obj(m));
  fun_add_method(rls, fun, m);
  stack_popn(rls, 2);
}

Method* fun_get_method(RlState* rls, Fun* fun, int argc) {
  Method* out = NULL;

  assert(fun->method != NULL);

  if ( is_singleton_fun(fun) && argc_match(fun->method, argc) )
      out = fun->method;

  else
    out = mtable_dispatch(rls, fun->methods, argc);

  return out;
}

void add_builtin_method(RlState* rls, Fun* fun, OpCode op, bool va, int arity, ...) {
  // NB: fun should already be defined at toplevel
  StackRef top = rls->s_top;
  Method* m = mk_builtin_method_s(rls, fun, arity, va, op);
  fun_add_method(rls, fun, m);
  rls->s_top = top;
}

void mtable_add(RlState* rls, MethodTable* mt, Method* m) {
  char* fname = mtable_name(mt);
  bool added = mtnode_add(rls, mt->root, m);

  require(rls, added, "conflicting signatures for %s/%d.", fname, m->arity);

  if ( m->nany == 0 && !m->va ) // safe to cache right away
    cache_method(rls, mt, m->signature, m);

  mt->mcount++;

  // update shortcut data
  if ( m->arity < mt->amin )
    mt->amin = m->arity;

  if ( m->arity > mt->amax )
    mt->amax = m->arity;

  if ( !mt->va && m->va )
    mt->va = true;
}

int order_methods(Method* mx, Method* my) {
  // signatures that specify more arguments are more specific
  int o = mx->arity - my->arity;

  // otherwise, a signature with more explicit types is more specific than one with none
  if ( o != 0 )
    o = mx->nexact - my->nexact;

  return o;
}

void save_candidate(Method* method, Method** candidate, Method** collision) {
  if ( *candidate == NULL ) {
    *candidate = method;
  } else {
    int o = order_methods(method, *candidate);

    if ( o > 0 ) { // method is a better match, save new candidate and resolve collision
      *candidate = method;
      *collision = NULL;
    } else if ( o == 0 ) { // indicate collision occurred
      *collision = method;
    } // do nothing
  }
}

bool mtnode_add(RlState* rls, MTNode* node, Method* method) {
  bool out;

  if ( method->arity == node->offset ) {
    if ( method->va ) {
      if ( node->vleaf == NULL ) {
        node->vleaf = method;
        out = true;
      } else {
        out = false;
      }
    } else {
      if ( node->fleaf == NULL ) {
        node->fleaf = method;
        out = true;
      } else {
        out = false;
      }
    }
  } else {
    int offset = node->offset;
    MTNode* child;
    Tuple* sig = method->signature;
    Type* stype = as_type(sig->data[offset]);

    if ( stype->tag == EXP_ANY ) {
      if ( node->fallback == NULL )
        node->fallback = mk_mtnode(rls, offset+1);

      child = node->fallback;
    } else {
      table_intern(rls, &node->children, stype, node, &child);
    }

    out = mtnode_add(rls, child, method);
  }

  return out;
}


DispatchResult mtnode_lookup(RlState* rls, MTNode* node, Tuple* sig, int argc, DispatchResult result, Method** candidate, Method** collision) {
  DispatchResult out = NO_MATCH; // -1 == no match, 0 = exact match, 1 = inexact match

  if ( node != NULL ) {
    int offset = node->offset;
    int siglen = sig->count;

    // more levels to traverse, but we might need to backtrack here
    if ( offset < siglen ) {
      Type* arg = as_type(sig->data[offset]);
      MTNode* child;

      // try exact
      if ( table_get(rls, &node->children, arg, &child) )
        out = mtnode_lookup(rls, child, sig, argc, result, candidate, collision);

      if ( out != EXACT_MATCH ) // could find a better candidate in the fallback table
        out = mtnode_lookup(rls, node->fallback, sig, argc, INEXACT_MATCH, candidate, collision);

      if ( out == NO_MATCH  && node->vleaf != NULL ) {
        out = result;
        save_candidate(node->vleaf, candidate, collision);
      }
    } else {
      if ( siglen == argc ) {
        if ( node->fleaf != NULL ) {
          save_candidate(node->fleaf, candidate, collision);
          out = result;
        } else if ( node->vleaf != NULL ) {
          save_candidate(node->vleaf, candidate, collision);
          out = result;
        } else {
          out = NO_MATCH;
        }
      } else {
        if ( node->vleaf != NULL ) {
          save_candidate(node->vleaf, candidate, collision);
          out = result;
        } else {
          out = NO_MATCH;
        }
      }
    }
  }

  return out;
}

Method* mtable_dispatch(RlState* rls, MethodTable* mt, int argc) {
  Method* out = NULL;
  Tuple* sig = NULL;
  Method* clash = NULL;
  int amin = mt->amin, amax = mt->amax;
  bool cached = false, vamt = mt->va;

  if ( argc >= amin && (argc <= amax || vamt) ) {
    int smax = min(argc, amax);
    sig = get_signature(rls, argc, smax);
    out = get_cached_method(rls, mt, sig);
    cached = out != NULL;

    if ( !cached )
      mtnode_lookup(rls, mt->root, sig, argc, EXACT_MATCH, &out, &clash);

    require(rls, clash == NULL, "ambiguous method call had multiple candidates.");
  }

  if ( !cached && out != NULL )
    cache_method(rls, mt, sig, out);

  return out;
}

// other dispatch APIs
Tuple* get_signature(RlState* rls, int o, int n) {
  // create a signature for a function call
  // basically inlining the process of hashing
  hash_t seed = hash_word_48(n+1);
  StackRef top = rls->s_top, base = top-o;

  for ( int i=0; i<n; i++ ) {
    Type* t = type_of(base[i]);
    seed = mix_hashes_48(seed, t->hashcode);
    stack_push(rls, tag_obj(t));
  }

  seed = mix_hashes_48(TupleType.hashcode, seed);
  Tuple* out = mk_tuple(rls, n);
  out->hashcode = seed;
  rls->s_top = top;
  return out;
}

hash_t mt_cache_hash(void* k) {
  return ((Tuple*)k)->hashcode;
}

hash_t mt_cache_rehash(KV* kv) {
  return ((Tuple*)kv->key)->hashcode;
}

bool mt_cache_compare(void* x, void* y) {
  // would prefer to turn this into a memcmp of an array of the
  // type identifiers but this at least short circuits multiple
  // levels of lookup, dispatch, and object access
  Tuple* tx = x, *ty = y;

  bool out = tx->count == ty->count;

  for ( int i=0; out && i < tx->count; i++ )
    out = as_type(tx->data[i])->tag == as_type(ty->data[i])->tag;

  return out;
}

void mt_cache_init(void* val, void* spc) {
  *(Method**)spc = val;
}

void mt_cache_mark(RlState* rls, Table* table, KV* kv) {
  if ( !table->weak_key )
    mark_obj(rls, kv->key);

  if ( !table->weak_val )
    mark_obj(rls, kv->val);
}

void init_mt_cache_table(RlState* rls, Table* table) {
  init_table(rls, table);
  table->sentinel = NULL;
  table->hash     = mt_cache_hash;
  table->rehash   = mt_cache_rehash;
  table->compare  = mt_cache_compare;
  table->init     = mt_cache_init;
  table->mark     = mt_cache_mark;
}

hash_t mt_node_hash(void* k) {
  return ((Type*)k)->hashcode;
}

hash_t mt_node_rehash(KV* kv) {
  return ((Type*)kv->key)->hashcode;
}

bool mt_node_compare(void* x, void* y) {
  return ((Type*)x)->tag == ((Type*)y)->tag;
}

void mt_node_mark(RlState* rls, Table* table, KV* kv) {
  if ( !table->weak_val )
    mark_obj(rls, kv->val);
}

void mt_node_init(void* val, void* spc) {
  *(MTNode**)spc = val;
}

void mt_node_intern(RlState* rls, Table* table, KV* kv, void* key, hash_t hash, void* state) {
  (void)table;
  (void)hash;

  MTNode* parent = state;
  kv->key = key;
  kv->val = mk_mtnode(rls, parent->offset+1);
}

void init_mt_node_table(RlState* rls, Table* table) {
  init_table(rls, table);
  table->sentinel = NULL;
  table->hash     = mt_node_hash;
  table->rehash   = mt_node_rehash;
  table->compare  = mt_node_compare;
  table->init     = mt_node_init;
  table->intern   = mt_node_intern;
  table->mark     = mt_node_mark;
}
