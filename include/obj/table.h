#ifndef rascal_table_h
#define rascal_table_h

#include "obj/type.h"

// table internal methods
typedef int        (*tbcompare_t)( rl_value_t x, rl_value_t y );
typedef size_t     (*tblength_t)( table_t *table );
typedef size_t     (*tbsize_t)( table_t *table );
typedef void       (*tbresize_t)( table_t *table, size_t newl );
typedef bool       (*tbref_t)(table_t *table, rl_value_t key, rl_value_t *buf );
typedef bool       (*tbset_t)(table_t *table, rl_value_t key, rl_value_t *buf, rl_value_t val );
typedef bool       (*tbput_t)(table_t *array, rl_value_t key, rl_value_t *buf );
typedef bool       (*tbpop_t)(array_t *array, rl_value_t key, rl_value_t *buf );

// generic table type (all table types can be safely cast to this type)
struct table_t
{
  BOX
  void       *data;
  size_t      length;
  size_t      capacity;
  function_t *compare;   // optional user compare
};

struct tb_impl_t
{
  size_t       entry_size;
  bool         unboxed_key;
  bool         unboxed_val;

  type_t      *keytype;
  type_t      *valtype;  // if none, table is not mapped

  // table methods
  tbcompare_t  tbcompare;
  tblength_t   tblength;
  tbsize_t     tbsize;
  tbresize_t   tbresize;
  tbref_t      tbref;
  tbset_t      tbset;
  tbput_t      tbput;
  tbpop_t      tbpop;
};

#endif
