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
  A*     new##A(flags_t fl);                                            \
  void   init##A(A* a);                                                 \
  void   free##A(void* p);                                              \
  size_t resize##A(A* a, size_t newCnt);                                \
  size_t _a##Push(A* a, X x);                                           \
  size_t _a##Write(A* a, size_t n, X* d);                               \
  size_t _a##PushN(A* a, size_t n, ...);                                \
  X      _a##Pop(A* a);                                                 \
  void   _a##PopN(A* a, size_t n)

ARRAY_TYPE(Binary8, byte_t);
ARRAY_TYPE(Binary16, uint16_t);
ARRAY_TYPE(Binary32, uint32_t);
ARRAY_TYPE(Buffer8, char);
ARRAY_TYPE(Buffer16, char16_t);
ARRAY_TYPE(Buffer32, char32_t);
ARRAY_TYPE(Alist, Value);
ARRAY_TYPE(Objects, Obj*);

// utility collection APIs
ARRAY_API(Binary8,  byte_t,   binary8);
ARRAY_API(Binary16, uint16_t, binary16);
ARRAY_API(Binary32, uint32_t, binary32);
ARRAY_API(Buffer8,  char,     buffer8);
ARRAY_API(Buffer16, char16_t, buffer16);
ARRAY_API(Buffer32, char32_t, buffer32);
ARRAY_API(Alist,    Value,    alist);
ARRAY_API(Objects, Obj*, objects);

#endif
