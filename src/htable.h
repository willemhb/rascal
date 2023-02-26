#ifndef htable_h
#define htable_h

#include "common.h"
#include "value.h"

// C types --------------------------------------------------------------------
typedef struct {
  void** table;
  usize count, cap;
} htable_t;

// API ------------------------------------------------------------------------
usize pad_array_size(usize newct, usize oldcap, usize mincap, double loadf);
usize pad_table_size(usize newct, usize oldcap);

#define ALIST(A, X)                             \
  typedef struct {                              \
    X* array;                                   \
    usize len, cap;                             \
  } TYPE(A)

#define HTABLE_API(type, ktype, vtype)                          \
  void    init_##type(htable_t* htable);                        \
  void    free_##type(htable_t* htable);                        \
  void    resize_##type(htable_t* htable, usize count);			\
  vtype   type##_get(htable_t* htable, ktype key);              \
  vtype   type##_set(htable_t* htable, ktype key, vtype val);   \
  vtype   type##_del(htable_t* htable, ktype key)

#define ALIST_API(A, X)                                         \
  void    init_##A(TYPE(A)* A);                                 \
  void    free_##A(TYPE(A)* A);                                 \
  void    resize_##A(TYPE(A)* A, usize len);                    \
  usize   A##_push(TYPE(A)* A, X x);                            \
  X       A##_pop(TYPE(A)* A);                                  \
  usize   A##_write(TYPE(A)* A, usize n, X* buf)

ALIST(bytes, ubyte);
ALIST(values, value_t);
ALIST(objects, object_t*);
ALIST(buffer, char);

ALIST_API(bytes, ubyte);
ALIST_API(values, value_t);
ALIST_API(objects, object_t*);
ALIST_API(buffer, char);

HTABLE_API(reader, int, funcptr);

#endif
