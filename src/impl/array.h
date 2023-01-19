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
  void *alloc##A(ObjectInit *args) {				      \
    assert(args->A##Init.nArgs > 0);				      \
    								      \
    args->A##Init.capacity  = padFn(args->A##Init.nArgs, 0);	      \
    args->size             += args->A##Init.capacity * sizeof(X);     \
    								      \
    return allocate(args->size);				      \
  }								      \
								      \
  void init##A(void *self, ObjectInit *args) {			      \
    ARRAY_HEAD(A, self)->length   = args->A##Init.nArgs;	      \
    ARRAY_HEAD(A, self)->capacity = args->A##Init.capacity;	      \
    X *xs                         = args->A##Init.array;	      \
    								      \
    memcpy(self, xs, ARRAY_HEAD(A, self)->length * sizeof(X));	      \
  }								      \
  								      \
  ARRAY_API(A, X)


#endif
