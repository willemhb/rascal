#include <tgmath.h>

#include "util/table.h"
#include "util/number.h"
#include "util/hashing.h"

#include "runtime.h"
#include "lang.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define LOAD_FACTOR 0.625
#define LOAD_PAD 1.6
#define MIN_CAP 8

// simple hash table type +++++++++++++++++++++++++++++++++++++++++++++++++++++
// misc utilities -------------------------------------------------------------
static value_t* allocate_table_data( usize cap ) {
  value_t* data = allocate(cap * 2 * sizeof(value_t), false);
  for ( usize i=0; i<cap; i++ )
    data[i*2] = data[i*2+1] = NOTFOUND;
  return data;
}

static void rehash_table( table_t* slf, usize newCap, value_t* newData ) {
  usize oldCap = slf->cap;
  value_t* oldData = slf->data;
  usize newMask = newCap - 1;

  for ( usize i=0; i<oldCap; i++ ) {
    value_t key = oldData[i*2], val = oldData[i*2+1];

    if ( key == NOTFOUND )
      continue;

    uhash keyHash = hash(key);
    uhash j = keyHash & newMask;

    while ( newData[j*2] != NOTFOUND )
      j = (j + 1) & newMask;

    newData[j*2] = key;
    newData[j*2+1] = val;
  }
}

static value_t* table_locate( table_t* slf, value_t key ) {
  usize cap = slf->cap;
  value_t* data = slf->data;
  usize mask = cap-1;
  usize keyHash = hash(key);
  usize i = keyHash & mask;

  while ( data[i*2] != NOTFOUND && compare(key, data[i*2]) )
    i = (i + 1) & mask;

  return &data[i*2];
}

void init_table( table_t* slf ) {
  slf->cnt = 0;
  slf->cap = MIN_CAP;
  slf->data = allocate_table_data(slf->cap);
}

void free_table( table_t* slf ) {
  deallocate(slf->data, 0, false);
  init_table(slf);
}

void reset_table( table_t* slf ) {
  free_table(slf);
}

usize resize_table( table_t* slf, usize n ) {
  usize pn = ceil(n * LOAD_PAD);
  
  if ( pn > slf->cap || pn < (slf->cap >> 1) ) {
    usize newCap = pn <= MIN_CAP ? MIN_CAP : ceil2(pn);
    value_t* newData = allocate_table_data(newCap);
    rehash_table(slf, newCap, newData);
    slf->cap = newCap;
    slf->data = newData;
  }

  return slf->cap;
}

value_t table_get( table_t* slf, value_t k ) {
  value_t* buf = table_locate(slf, k);
  return buf[1];
}

value_t table_set( table_t* slf, value_t k, value_t v ) {
  resize_table(slf, slf->cnt+1);

  value_t* buf = table_locate(slf, k);
  value_t out = buf[1];

  if ( *buf == NOTFOUND ) {
    slf->cnt++;
    *buf = k;
  }

  buf[1] = v;
  return out;
}

value_t table_add( table_t* slf, value_t k, value_t v ) {
  resize_table(slf, slf->cnt+1);

  value_t* buf = table_locate(slf, k);
  value_t out = buf[1];

  if ( *buf == NOTFOUND ) {
    slf->cnt++;
    buf[0] = k;
    buf[1] = v;
  }

  return out;
}

value_t table_del( table_t* slf, value_t k ) {
  value_t* buf = table_locate(slf, k);
  value_t out = buf[1];

  if ( *buf != NOTFOUND ) {
    buf[0] = NOTFOUND;
    buf[1] = NOTFOUND;
    slf->cnt--;
    resize_table(slf, slf->cnt);
  }

  return out;
}
