#include <string.h>

#include "lang/dispatch.h"
#include "util/util.h"
#include "vm.h"
#include "val.h"

// C types --------------------------------------------------------------------
// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
Method* get_cached_method(RlState* rls, MethodTable* mt, Tuple* sig);
void cache_method(RlState* rls, MethodTable* mt, Tuple* sig, Method* method);
int order_methods(Method* m, Method* my);
void mtnode_lookup(RlState* rls, MTNode* node, Tuple* sig, int argc, Method** candidate, Method** collision);

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

void add_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  // NB: fun should already be defined at toplevel
  StackRef top = rls->s_top;
  Method* m = mk_builtin_method_s(rls, fun, arity, va, op);
  fun_add_method(rls, fun, m);
  rls->s_top = top;
}

void mtable_add(RlState* rls, MethodTable* mt, Method* m) {
  char* fname = mtable_name(mt);
  // check for duplicate signature
}

void mtnode_lookup(RlState* rls, MTNode* node, Tuple* sig, int argc, Method** candidate, Method** collision) {
  if ( node == NULL )
    return;

  Type* arg = as_type(sig->data[node->offset]);
  
}

Method* mtable_dispatch(RlState* rls, MethodTable* mt, int argc) {
  Method* out = NULL;

  // simplest case of no arguments
  if ( argc == 0 ) {
    out = mt->fthunk ? : mt->vthunk;
  } else {
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
        mtnode_lookup(rls, mt->root, sig, argc, &out, &clash);

      require(rls, clash == NULL, "ambiguous method call had multiple candidates.");

      if ( out == NULL )
        out = mt->vthunk; // final fallback
      }

    if ( !cached && out != NULL )
      cache_method(rls, mt, sig, out);
  }
 
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

void init_mt_node_table(RlState* rls, Table* table) {
  init_table(rls, table);
  table->sentinel = NULL;
  table->hash     = mt_node_hash;
  table->rehash   = mt_node_rehash;
  table->compare  = mt_node_compare;
  table->mark     = mt_node_mark;
}
