#ifndef rascal_tpl_table_h
#define rascal_tpl_table_h

#include "tpl/impl/type.h"

/* API signatures */
#define MAKE_TABLE( T, K, V )				\
  T make_##T( type_t type, size_t n, void *ini )

#define FREE_TABLE( T, K, V )			\
  void free_##T( TYPE(T) T )

#define RESIZE_TABLE( T, K, V )				\
  size_t resize_##T( TYPE(T) T, size_t new_count )

#define TABLE_GET( T, K, V )			\
  bool T##_get( TYPE(T) T, K k, V *buf )

#define TABLE_SET( T, K, V )			\
  bool T##_set( TYPE(T) T, K k, V *buf )

#define TABLE_PUT( T, K, V )			\
  bool T##_put( TYPE(T) T, K k, V *buf )

#define TABLE_POP( T, K, V )			\
  bool T##_put( TYPE(T) T, K k, V *buf )

#endif
