#ifndef rascal_tpl_impl_alist_h
#define rascal_tpl_impl_alist_h

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../util/collection.h"

#undef ALIST
#undef ALIST_OBJECT

#define ALIST(A, X, padFn)						\
									\
  static void resize##A(A *array, int newSize) {			\
    int oldCap    = array->capacity;					\
    int newCap    = padFn(newSize, oldCap);				\
    if (newCap != oldCap) {						\
      X *arr          = array->array;					\
      array->array    = reallocArr(arr, oldCap, newCap, sizeof(X));	\
      array->array    = arr;						\
      array->capacity = newCap;						\
    }									\
    array->length = newSize;						\
  }									\
									\
  A *create##A(void) {							\
    return allocate(sizeof(struct A));					\
  }									\
									\
  void destroy##A(A *array) {						\
    free##A(array);							\
    deallocate(array, sizeof(struct A));				\
  }									\
									\
  void init##A(A *array) {						\
    array->length   = 0;						\
    array->capacity = 0;						\
    array->array    = NULL;						\
  }									\
  									\
  void free##A(A *array) {						\
    deallocArr(array->array, array->capacity, sizeof(X));		\
    init##A(array);							\
  }									\
  									\
  X *A##Peep(A *array, int i) {						\
    if (i < 0)								\
      i += array->length;						\
    ASSERT_INDEX(i, array->length);					\
    return array->array + i;						\
  }									\
  									\
  X A##Get(A *array, int i) {						\
    return *A##Peep(array, i);						\
  }									\
  									\
  X A##Set(A *array, int i, X x) {					\
    *A##Peep(array, i) = x;						\
    return x;								\
  }									\
									\
  int A##Push(A *array, X x) {						\
    resize##A(array, array->length+1);					\
    array->array[array->length-1] = x;					\
    return array->length;						\
  }									\
  									\
  X *A##Write(A *array, X *xs, int l) {					\
    ASSERT_BOUND(l, 0, INT32_MAX);					\
    int offset = array->length;						\
    resize##A(array, offset+l);						\
    X *dst = A##Peep(array, offset);					\
    if (array)								\
      memcpy(dst, xs, l * sizeof(X));					\
    return dst;								\
  }									\
  									\
  X A##Pop(A *array) {							\
    assert(array->length > 0);						\
    X out = array->array[array->length-1];				\
    resize##A(array, array->length-1);					\
    return out;								\
  }



#endif

