#ifndef rascal_impl_array_h
#define rascal_impl_array_h

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../runtime.h"

#include "../util/collection.h"
#include "../util/number.h"

#undef ARRAY
#undef ARRAY_OBJECT
#undef ARRAY_API

#define ARRAY_API(A, X)						    \
  								    \
  X *A##Peep(A array, int i) {                                      \
    struct A *head = ARRAY_HEAD(A, array);                          \
    if (i < 0)                                                      \
      i += head->length;                                            \
    ASSERT_INDEX(i, head->length);				    \
    return array + i;                                               \
  }                                                                 \
                                                                    \
  X A##Get(A array, int i) {                                        \
    return *A##Peep(array, i);                                      \
  }                                                                 \
                                                                    \
  X A##Set(A array, int i, X x) {                                   \
    *A##Peep(array, i) = x;                                         \
    return x;                                                       \
  }                                                                 \
                                                                    \
  int A##Write(A array, X *xs, int start, int count) {              \
    struct A *head = ARRAY_HEAD(A, array);                          \
    if (start >= head->length)                                      \
      return 0;                                                     \
                                                                    \
    count = min(head->length - start, count);                       \
                                                                    \
    if (count > 0)                                                  \
      memcpy(array+start, xs, count * sizeof(X));                   \
                                                                    \
    return count;                                                   \
  }

#define ARRAY(A, X, padFn)                                          \
  A create##A(int length) {                                         \
    ASSERT_BOUND(length, 0, INT32_MAX);                             \
    int capacity  = padFn(length, 0);                               \
    usize base    = sizeof(struct A);                               \
    usize array   = capacity * sizeof(X);                           \
    struct A* out = allocate(base+array);                           \
    out->length   = length;                                         \
    out->capacity = capacity;                                       \
    return out->array;                                              \
  }                                                                 \
                                                                    \
  void destroy##A(A array) {                                        \
    struct A *head = ARRAY_HEAD(A, array);                          \
    usize arrSize  = sizeof(struct A) + sizeof(X) * head->capacity; \
    deallocate(head, arrSize);                                      \
  }                                                                 \
                                                                    \
  void init##A(A array, X *data) {                                  \
    assert(data);                                                   \
    struct A *head = ARRAY_HEAD(A, array);                          \
    memcpy(array, data, head->length * sizeof(X));                  \
  }								    \
  								    \
  ARRAY_API(A, X)

#define ARRAY_OBJECT(A, X, padFn, EmptyCapacity, EmptyInit...)	      \
								      \
  struct A Empty##A = {						      \
    .length=0,							      \
    .capacity=EmptyCapacity,					      \
    .obj = {							      \
      .next=NULL,						      \
      .hash=0,							      \
      .type=A##Type,						      \
      .flags=0,							      \
      .hashed=false,						      \
      .lendee=false,						      \
      .inlined=true,						      \
      .allocated=false,						      \
      .gray=false,						      \
      .black=true,						      \
      .offset=sizeof(struct A),					      \
      .size=sizeof(struct A) + EmptyCapacity * sizeof(X),	      \
    },								      \
    .array = { EmptyInit }					      \
  };								      \
								      \
  struct A##Args {						      \
    int length;							      \
    int capacity;						      \
    X  *values;							      \
  };								      \
								      \
  usize alloc##A(RlType type, void *args, void **dst) {		      \
    (void)type;							      \
    struct A##Args *arrayArgs = args;				      \
								      \
    assert(arrayArgs->length > 0);				      \
    int length   = arrayArgs->length;				      \
    int capacity = padFn(length, 0);				      \
    usize total  = sizeof(struct A) + capacity * sizeof(X);	      \
    void *spc    = allocate(total);				      \
								      \
    ((struct A*)spc)->length      = length;			      \
    ((struct A*)spc)->capacity    = capacity;			      \
    ((struct A*)spc)->obj.inlined = true;			      \
    ((struct A*)spc)->obj.lendee  = false;			      \
								      \
    *dst = spc;							      \
    arrayArgs->capacity = capacity;				      \
    return total;						      \
  }								      \
								      \
  void init##A(void *self, RlType type, void *args) {		      \
    (void)type;							      \
    struct A##Args *arrayArgs =	args;				      \
    memcpy(self, arrayArgs->values, arrayArgs->capacity * sizeof(X)); \
  }								      \
  								      \
  ARRAY_API(A, X)


#endif
