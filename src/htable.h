#ifndef htable_h
#define htable_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef struct {
  void** table;
  usize count, cap;
} htable_t;

// API ------------------------------------------------------------------------
#define HTABLE_API(type, ktype, vtype)					\
  void    init_##type(htable_t* htable);				\
  void    free_##type(htable_t* htable);				\
  void    resize_##type(htable_t* htable, usize count);			\
  vtype   type##_get(htable_t* htable, ktype key);			\
  vtype   type##_set(htable_t* htable, ktype key, vtype val);		\
  vtype   type##_del(htable_t* htable, ktype key)

HTABLE_API(reader, int, funcptr);

#endif
