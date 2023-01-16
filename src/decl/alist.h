#ifndef rascal_decl_alist_h
#define rascal_decl_alist_h

#include "../common.h"

#define ALIST_API(T, X)                          \
  void init##N(T *array);                        \
  void free##N(T *array);                        \
  void resize##N(T *array);                      \
  int  N##Push(T *array, X x);                   \
  int  N##Write(T *array, X *xs, int n);         \
  X    N##Pop(T *array);                         \
  X    N##Get(T *array, int i);                  \
  X   *N##Peep(T *array, int i);                 \
  X    N##Set(T *array, int i, X x)

#define ALIST(T, X)                              \
  typedef struct X {                             \
    int length;                                  \
    int capacity;                                \
    X  *array;                                   \
  } T;                                           \
  ALIST_API(T, X)

#define ALIST_OBJECT(T, X)                      \
  typedef struct T {                            \
    int length;                                 \
    int capacity;                               \
    struct Object obj;                          \
    X  *array;                                  \
  } T;                                          \
  ALIST_API(T, X)

#endif
