#ifndef rascal_decl_alist_h
#define rascal_decl_alist_h

#include "../common.h"


#define ALIST(A, X)					 \
  typedef struct A {					 \
    X  *array;						 \
    int length;						 \
    int capacity;					 \
  } A;							 \
							 \
  A   *create##A(void);					 \
  void destroy##A(A *array);				 \
  void init##A(A *array);				 \
  void free##A(A *array);				 \
  /* access/mutate */					 \
  X    A##Get(A *array, int i);				 \
  X   *A##Peep(A *array, int i);			 \
  X    A##Set(A *array, int i, X x);			 \
  X   *A##Write(A *array, X *xs, int count);		 \
  int  A##Push(A *array, X x);				 \
  X    A##Pop(A *array )

#endif
