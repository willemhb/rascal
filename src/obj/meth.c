#include "obj/meth.h"

/* hashing utilities */
#include "utils/str.h"
#include "utils/num.h"
#include "utils/arr.h"
#include "utils/table.h"

/* core */
#include "mem.h"

/* common implementation */
meth_t *cache_method(metht_t *metht, type_t t, hash_t h, )
{
  
}

meth_t *insert_method()
{
  
}

/* method table implementation */
static const real_t mcache_load = 0.75;
static const size_t mcache_min_cap = 32;
 
static inline bool mcache_guard(meth_t* e) { return e == NULL; }
static inline bool mcache_cmph(hash_t h, meth_t *e) { return h == e->sig.hash; }
static inline bool mcache_cmpk(sig_t *s, meth_t *e)
{
  return s->len == e->sig.len
    && u32cmp(s->data, e->sig.data, s->len ) == 0;
}

static inline hash_t mcache_ehash(meth_t *m)
{
  return m->sig.hash;
}

static inline bool mcache_khash(sig_t *s) { return s->hash; }

TABLE_PAD(mcache);
TABLE_TRACE(mcache, obj_t*);
TABLE_INIT(mcache, meth_t*);
TABLE_FINALIZE(mcache, meth_t*, obj_t*);
TABLE_RESIZE(mcache, meth_t*);
TABLE_REHASH(mcache, meth_t*, mcache_guard, mcache_ehash);

void trace_metht(metht_t *metht)
{
  trace_mcache(&metht->cache);

  mark((obj_t*)metht->func);
  mark((obj_t*)metht->tree);
}

TABLE_GET(mcache, meth_t*, sig_t*, mcache_guard, mcache_cmph, mcache_cmpk, mcache_khash);

/* method implementaiton */
static const real_t mlevel_load = 0.75;
static const size_t mlevel_min_cap =  1;

static inline bool mlevel_guard(meth_t *e) { return e == NULL; }

#define mlevel_cmph(t, e) true

static inline bool mlevel_ehash(meth_t *e)
{
  return e->sig.data[e->sig.len-1];
}

static inline bool mlevel_cmpk(type_t t, meth_t *e)
{
  return t == mlevel_ehash(e);
}

#define mlevel_khash(t) (t)

TABLE_PAD(mlevel);
TABLE_TRACE(mlevel, obj_t*);
TABLE_INIT(mlevel, meth_t*);
TABLE_FINALIZE(mlevel, meth_t*, obj_t*);
TABLE_RESIZE(mlevel, meth_t*);
TABLE_REHASH(mlevel, meth_t*, mlevel_guard, mlevel_ehash);


void trace_meth(meth_t *meth)
{
  trace_mlevel(&meth->children);

  if (flag_p(meth->children.object.flags, meth_fl_bytecode))
    mark((obj_t*)meth->bytecode);

  /* while unlikely its possible these haven't been visited if methods are shared
     (i'm not sure why they would be but who wants to think about this right now?)
   */
  mark((obj_t*)meth->ancestor);
  mark((obj_t*)meth->parent);
}

TABLE_GET(mlevel, meth_t*, type_t, mlevel_guard, mlevel_cmph, mlevel_cmpk, mlevel_khash);
