#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "hashing.h"

#include "table.h"

#include "object.h"
#include "runtime.h"
#include "io.h"

#include "symbol.h"
#include "array.h"
#include "number.h"
#include "list.h"
#include "function.h"

// forward declarations -------------------------------------------------------
static bool table_is_terminal(table_t *t, index_t i);

typedef struct {
  size_t  bufi;
  index_t path;
  value_t buf[16];
} table_buffer_t;

static void init_table_buffer(table_buffer_t *b) {
  b->bufi = 0;
  b->path = 0;
}

static void table_buffer_put(table_buffer_t *b, index_t i, value_t v) {
  assert(b->bufi < 16);

  b->path |= i << b->bufi*4;
  b->buf[b->bufi++] = v;
}

static inline index_t node_index(table_t *t, index_t i) {
  return popcnt(t->bmap & ((1 << i)-1));
}

static inline size_t node_length(table_t *t) {
  return popcnt(t->bmap);
}

static inline size_t node_offset(table_t *t) {
  return t->depth * 4 + 4;
}

static inline index_t hash_to_index(table_t *t, hash_t h) {
  return (h >> node_offset(t)) & 15;
}

static inline bool table_is_bound(table_t *t, index_t i) {
  return !!(t->bmap & (1<<i));
}

static cons_t *table_get_entry(table_t *t, index_t i) { 
  return t->data + node_index(t, i);
}

static value_t table_get_keyval(table_t *t, index_t i) {
  assert(table_is_bound(t, i));

  return table_get_entry(t, i)->car;
}

static hash_t table_get_hash(table_t *t, index_t i) {
  assert(table_is_bound(t, i));

  return table_get_entry(t, i)->cdr;
}

static value_t table_get_key(table_t *t, index_t i) {
  assert(table_is_bound(t, i));

  return car( table_get_entry(t, i)->car );
}

static value_t table_get_bind(table_t *t, index_t i) {
  assert(table_is_bound(t, i));

  return cdr( table_get_entry(t, i)->car );
}

static bool table_is_collision(table_t *t, index_t i) {
  return (t->children & (1<<i)) && is_cons(table_get_key(t, i));
}

static bool table_is_subtable(table_t *t, index_t i) {
  return (t->children & (1<<i)) && is_table(table_get_key(t, i));
}

static bool table_is_terminal(table_t *t, index_t i) {
  return (t->bmap & (1<<i)) && !(t->children & (1<<i));
}

static bool is_hash_match(table_t *t, index_t i, hash_t h) {
  assert(table_is_bound(t, i));

  value_t k1 = table_get_key(t, i);
  hash_t  h1 = table_get_hash(t, i);
  
  if ((t->bmap & (1<<i)) && is_table(k1))
    h &= node_offset(pval(k1)) - 1;

  return h == h1;
}

static size_t table_length(value_t x) {
  if (is_empty(x))
    return 0;

  table_t *t = pval(x);
  size_t n = popcnt(t->bmap) - popcnt(t->children);
  size_t i = ffs(t->children);

  while (i) {
    value_t k = table_get_key(t, i-1);

    if (is_cons(k))
      n += list_length(k);

    else
      n += table_length(k);

    index_t m = ~(( 1 << i)-1);

    i = ffs(t->children & m); // exclude already traversed nodes
  }

  return n;
}

static void cache_put(table_t *xt, value_t xc ) {
  value_t *xp = &xt->cache;

  while (is_cons(*xp)) {
    if (car(*xp) < car(xc))
      xp = &cdr(*xp);

    else
      break;
  }

  cdr(xc) = *xp;
  *xp     =  xc;
}

static value_t cache_pop( value_t x, size_t n ) {
  assert(n <= 16);

  if (is_empty(x))
    return val_nil;

  table_t *xt = pval(x);
  value_t *xp = &xt->cache, out = val_nil;

  while (is_cons(*xp)) {
    if (uval(car(*xp)) < n)
      xp = &cdr(*xp);

    else {
       out = *xp;
      *xp  = cdr(*xp);
      break;
    }
  }

  return out;
}

static value_t dict_grow(value_t xd) {
  if (xd == val_edict)
    return dict( 1, NULL);
  
  size_t dl = node_length(pval(xd));
  size_t ds = nextipow2(dl);
  size_t ns = ds * 2;

  assert(ns <= 16);

  push(xd);
  cons_t *nspc = allocate(ns*sizeof(cons_t));

  xd = pop();
  cons_t *swap = tdata(xd);
  /* copy entries */
  memcpy( nspc, swap, dl*sizeof(cons_t));

  tdata(xd)  = nspc;
  swap->car  = fixnum(ds);
  swap->cdr  = val_nil;
  cache_put(pval(xd), tagp(swap, tag_pair));
  return xd;
}

static value_t set_grow(value_t xs) {
  if (xs == val_edict)
    return set( 1, NULL);
  
  size_t sl = node_length(pval(xs));
  size_t ss = nextipow2(sl);
  size_t ns = ss * 2;

  assert(ns <= 16);

  push(xs);
  cons_t *nspc = allocate(ns*sizeof(cons_t));

  xs = pop();
  cons_t *swap = tdata(xs);
  /* copy entries */
  memcpy( nspc, swap, sl*sizeof(cons_t));

  tdata(xs)  = nspc;
  swap->car  = fixnum(ss);
  swap->cdr  = val_nil;
  cache_put(pval(xs), tagp(swap, tag_pair));

  return xs;
}

static value_t table_locate(value_t xt, value_t k, hash_t h, table_buffer_t *buf) {
  if (is_empty(xt))
    return val_nil;

  table_t *t = pval(xt);
  index_t i = hash_to_index(t, h);

  if (!table_is_bound(t, i))
    return val_nil;

  value_t kb1 = table_get_keyval(t, i);
  value_t k1  = car( kb1 );

  if (buf != NULL)
    table_buffer_put(buf, i, k1 );

  if (!is_hash_match(t, i, h))
    return val_nil;

  if (table_is_collision(t, i))
    return cons_assoc(k1, k);
  
  else if (table_is_subtable(t, i))
    return table_locate(k1, k, h, buf);
 
  else if (r_equals(k, k1))
    return kb1;

  else
    return val_nil;
}

value_t table_assoc(value_t xt, value_t k) {
  hash_t h = r_hash(k);
  return table_locate(xt, k, h, NULL);
}

value_t dict_put(value_t *xd, value_t k) {
  if (is_empty(*xd)) {
    push(k);
    push(val_nil);
    *xd = dict(1, &Sref(2));
    popn(2);
    return tdata(*xd)[0].car;
  }

  table_buffer_t buf;
  init_table_buffer(&buf);

  hash_t h = r_hash(k);

  value_t location = table_locate(*xd, k, h, &buf);

  if (location != val_nil)
    return location;

  
}
