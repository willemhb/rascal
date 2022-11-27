#ifndef rascal_tpl_table_h
#define rascal_tpl_table_h

#include "tpl/impl/type.h"

// convenience macros
#define TABLE( E )				\
  OBJHEAD;					\
  size_t   count;				\
  size_t   capacity;				\
  E      **entries

#define MAKE_TABLE( T, K, V )					\
  T make_##T( type_t type, size_t n, void *ini )		\
  {								\
    								\
  }

#define FREE_TABLE( T, K, V )			\

#define RESIZE_TABLE( T, K, V )			\

#define TABLE_LOOKUP( T, K, V )			\

#define TABLE_INTERN( T, K, V )			\



#endif
