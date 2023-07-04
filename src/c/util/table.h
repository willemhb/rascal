#ifndef util_table_h
#define util_table_h

#include "common.h"

// simple hash table type +++++++++++++++++++++++++++++++++++++++++++++++++++++
struct table {
  value_t* data;
  usize cnt, cap;
};

void init_table( table_t* slf );
void free_table( table_t* slf );
void reset_table( table_t* slf );
usize resize_table( table_t* slf, usize n );
value_t table_get( table_t* slf, value_t k );
value_t table_add( table_t* slf, value_t k, value_t v );
value_t table_set( table_t* slf, value_t k, value_t v );
value_t table_del( table_t* slf, value_t k );
uhash hash_table( table_t* slf );
void mark_table( table_t* slf );

#endif
