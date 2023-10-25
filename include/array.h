#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

/* utility array types */

#define ARRAY_TYPE(A, X)       \
                               \
                               \
  struct A {                   \
    Obj    obj;                \
    X*     data;               \
    size_t cnt;                \
    size_t cap;                \
  };                           \
                               \
  extern struct Type A##Type

#define ARRAY_API(A, X, _a)                                             \
  A*     new_##_a(flags_t fl);                                          \
  void   init_##_a(A* a);                                               \
  void   free_##_a(void* p);                                            \
  size_t resize_##_a(A* a, size_t new_cnt);                             \
  X*     _a##_peek(A* a, int i);                                        \
  size_t _a##_push(A* a, X x);                                          \
  size_t _a##_write(A* a, size_t n, X* d);                              \
  size_t _a##_pushn(A* a, size_t n, ...);                               \
  X      _a##_pop(A* a);                                                \
  void   _a##_popn(A* a, size_t n)

ARRAY_TYPE(Binary8,  byte_t);
ARRAY_TYPE(Binary16, uint16_t);
ARRAY_TYPE(Binary32, uint32_t);
ARRAY_TYPE(Buffer8,  char);
ARRAY_TYPE(Buffer16, char16_t);
ARRAY_TYPE(Buffer32, char32_t);
ARRAY_TYPE(Alist,    Value);
ARRAY_TYPE(Objects,  Obj*);

// utility collection APIs
ARRAY_API(Binary8,  byte_t,   binary8);
ARRAY_API(Binary16, uint16_t, binary16);
ARRAY_API(Binary32, uint32_t, binary32);
ARRAY_API(Buffer8,  char,     buffer8);
ARRAY_API(Buffer16, char16_t, buffer16);
ARRAY_API(Buffer32, char32_t, buffer32);
ARRAY_API(Alist,    Value,    alist);
ARRAY_API(Objects,  Obj*,     objects);

#endif
