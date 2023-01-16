#ifndef rascal_decl_array_h
#define rascal_decl_array_h

#include "../common.h"

#define ARRAY_API(T, X)                          \
  T new##T(usize n);                             \
  void init##T(T array, X *data);                \
  void free##T(T array);                         \
  T resize##T(T array, usize newLength);         \
  X *T##Peep(T array, int i);                    \
  X T##Get(T array, int i);                      \
  X T##Set(T array, int i, X x);                 \
  int T##Push(T array, X x);                     \
  int T##Write(T array, X *xs, int n);           \
  X T##Pop(T array)

#define ARRAY(T, X)                                                     \
  /* define array as pointer to elements */                             \
  typedef X *T;                                                         \
                                                                        \
  struct T {                                                            \
    int length;                                                         \
    int capacity;                                                       \
    X elements[];                                                       \
  };                                                                    \
  ARRAY_API(T, X)

#define ARRAY_OBJECT(T, X)                       \
  typedef X *T;                                  \
  struct T {                                     \
    int length;                                  \
    int cap;                                     \
    struct Object obj;                           \
    X elements[];                                \
  };                                             \
  ARRAY_API(T, X)


#endif
