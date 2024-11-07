#include <math.h>

#include "vm/environ.h"
#include "vm/heap.h"

#include "val/text.h"

#include "util/number.h"
#include "util/hash.h"
#include "util/text.h"

/* Globals */
StrTable Strings = {
  .cnt  = 0,
  .cap  = 0,
  .strs = NULL
};

#define ST_LF 0.625
#define ST_PF 1.6

/* Internal APIs */
static size64 pad_str_table_size(size64 n) {
  if ( n == 0 )
    return 0;

  size64 pad = ceil(n * ST_PF);

  if ( pad < 8 )
    pad = 8;

  else
    pad = ceil2(pad);

  return pad;
}

static Str** alloc_str_table_strs(size64 p) {
  return rl_alloc(NULL, p * sizeof(Str*));
}

static bool check_str_table_resize(size64 n, size64 c) {
  return n >= c * ST_PF;
}

static Str** find_str_table_entry(StrTable* st, char* cs, hash64 ch) {
  Str**  strs = st->strs;
  size64 cap  = st->cap;
  size64 msk  = cap - 1;
  size64 idx  = ch & msk;
  Str**  loc  = NULL;

  for (;;) {
    loc = &strs[idx];

    if ( *loc == NULL )
      break;

    else if ( ch == (*loc)->chash && seq(cs, (*loc)->cs) )
      break;

    else
      idx = (idx + 1) & msk;
  }

  return loc;
}

static void rehash_str_table(StrTable* st, Str** os, size64 oc) {
  // 
  st->cnt    = 0;
  st->nts    = 0;

  // 
  Str**  ns  = st->strs;
  size64 nc  = st->cap;
  size64 nm  = nc - 1; // table size mask

  for ( size64 i=0; i < oc; i++ ) {
    Str* s = os[i];

    if ( s == NULL )
      continue;

    else if ( s->cs == NULL ) { // tombstone
      sweep_obj(st->vm, s);
      continue;
    }

    else {
      size64 sh  = s->chash;
      size64 idx = sh & nm;

      while ( ns[idx] )
        idx = ( idx + 1 ) & nm;

      ns[idx] = s;
      st->cnt++;
    }
  }
}

/* External APIs */
// StrTable API
void init_str_table(StrTable* st) {
  st->cnt  = 0;
  st->cap  = 0;
  st->nts  = 0;
  st->strs = NULL;
}

void free_str_table(StrTable* st) {
  rl_dealloc(NULL, st->strs, 0);
  init_str_table(st);
}

void resize_str_table(StrTable* st, size64 n) {
  if (n == 0) {
    free_str_table(st);
  } else {
    size64 nsize = pad_str_table_size(n);
    Str**  nstrs = alloc_str_table_strs(nsize);
    
    if ( st->strs == NULL ) {
      st->strs = nstrs;

    } else { // needs rehash
      // get old table and size
      Str**  ostrs = st->strs;
      size64 osize = st->cap;
      st->strs     = nstrs;
      st->cap      = nsize;

      // rehash table
      rehash_str_table(st, ostrs, osize);

      // free the old table
      rl_dealloc(NULL, ostrs, 0);
    }

    // set the new table capacity
    st->cap = nsize;
  }
}

void sweep_str_table(StrTable* st) {
  if ( st->nts > 0 ) {
    // save old strings for rehash and allocate new entries
    Str**  os = st->strs;
    st->strs  = alloc_str_table_strs(st->cap);

    // rehash the table to remove tombstones
    rehash_str_table(st, os, st->cap);

    // free the old entries
    rl_dealloc(NULL, os, 0);
  }
}

Str* intern_str(StrTable* st, char* cs, size64 n) {
  if ( check_str_table_resize(st->cnt+1, st->cap) )
    resize_str_table(st, st->cnt+1);

  hash64 ch  = hash_chars(cs, n);
  Str**  loc = find_str_table_entry(st, cs, ch);

  if ( *loc == NULL ) {
    st->cnt++;
    *loc = new_str(cs, n, true, ch);
  }

  return *loc;
}
