#ifndef rascal_decl_array_h
#define rascal_decl_array_h

#include "../common.h"

#define ARRAY_API(A, X)                                 \
  /* construct/destruct */                              \
  X  *A##Peep(A array, int i);                          \
  X   A##Get(A array, int i);                           \
  X   A##Set(A array, int i, X x);                      \
  int A##Write(A array, X *xs, int start, int count)

#define ARRAY(A, X)                                                     \
  /* define array as pointer to elements */                             \
  typedef X *A;                                                         \
                                                                        \
  struct A {                                                            \
    int length;                                                         \
    int capacity;                                                       \
    X   array[];                                                        \
  };                                                                    \
  									\
  A create##A(int length);						\
  void destroy##A(A array);						\
  void init##A(A array, X *data);					\
  ARRAY_API(A, X)

#define ARRAY_OBJECT(A, X)				\
  typedef X *A;						\
  struct A {						\
    int length;						\
    int capacity;					\
    struct Object obj;					\
    X   array[];					\
  };							\
							\
  extern struct A Empty##A;				\
							\
  usize alloc##A(RlType type, void *args, void **dst);	\
  void  init##A(void *self, RlType type, void *args);	\
  ARRAY_API(A, X)

#define ARRAY_HEAD(A, a)				\
  ((struct A*)(((ubyte*)(a)) - sizeof(struct A)))

#endif
