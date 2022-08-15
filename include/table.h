#ifndef rascal_table_h
#define rascal_table_h

#include "common.h"

// C types --------------------------------------------------------------------
struct entry_t {
  value_t key;
  hash_t hash;
};

struct hamt_t {
  uint_t   bitmap;
  uchar_t  depth;
  uchar_t  length;
  ushort_t tag;

  value_t  cache;
  value_t  data[0];
};

struct table_t {
  uint_t   length;
  uchar_t  keytype;
  uchar_t  bindtype;
  uchar_t  cmptype;
  uchar_t  maptype;
  ushort_t tag;

  value_t  entries;
};

// utilities ------------------------------------------------------------------
value_t table_ref(value_t t, void *k);
value_t vector_ref(value_t v, uint_t i);
value_t dict_ref(value_t t, value_t k);
value_t set_ref(value_t s, value_t k);

// native functions -----------------------------------------------------------
value_t native_dict(value_t args, size_t n_args);
value_t native_set(value_t args, size_t n_args);
value_t native_table(value_t args, size_t n_args);
value_t native_vector(value_t args, size_t n_args);

// initialization -------------------------------------------------------------
void table_init(void);

#endif
