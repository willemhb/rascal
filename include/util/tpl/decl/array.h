#ifndef rascal_tpl_array_h
#define rascal_tpl_array_h

#include "obj/object.h"
#include "tpl/impl/type.h"

/* generic array type */
struct array_t
{
  size_t len;
  size_t cap;
  struct object_t obj;
};

/* array type template */
#define ARRAY( A, X )				\
  struct TYPE(A)				\
  {						\
    struct array_t array;			\
    X data[];					\
  }

/* array macro templates */
#define ARRAY_HEADER( A, x ) ((struct TYPE(A)*)obj_start(x))
#define ARRAY_LEN( A, x )    (ARRAY_HEADER(A, x)->array.len)
#define ARRAY_CAP( A, x )    (ARRAY_HEADER(A, x)->array.cap)

/* array convenience functions */

/* array API signatures */
#define MAKE_ARRAY( A, X )				\
  TYPE(A) make_##A( type_t eltype, size_t len, X *ini )

#define RESIZE_ARRAY( A, X )				\
  TYPE(A) resize_##A( TYPE(A) A, size_t new_len )

#define ARRAY_REF( A, X )			\
  X A##_ref( TYPE(A) A, long i )

#define ARRAY_SET( A, X )			\
  X A##_set( TYPE(A), long i, X x )

#endif
