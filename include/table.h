#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"

// convenience macros
#define TABLE( E )				\
  OBJHEAD;					\
  size_t   count;				\
  size_t   capacity;				\
  E      **entries

// C types
struct table_t
{
  TABLE(cons_t);
};

#endif
