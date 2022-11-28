#ifndef rascal_tpl_string_h
#define rascal_tpl_string_h

#include "obj/object.h"
#include "tpl/impl/type.h"

/* generic string type */
struct string_t
{
  size_t          len;
  size_t          cap;
  struct object_t obj;
};

/* string type template */
#define STRING( A, X )				\
  struct TYPE(A)				\
  {						\
    struct string_t string;			\
    X data[];					\
  }

/* string macro templates */
#define STRING_HEADER( A, x ) ((struct TYPE(A)*)obj_start(x))
#define STRING_LEN( A, x )    (STRING_HEADER(A, x)->string.len)
#define STRING_CAP( A, x )    (STRING_HEADER(A, x)->string.cap)

/* string convenience functions */

/* string API signatures */
#define MAKE_STRING( A, X )				\
  TYPE(A) make_##A( type_t eltype, size_t len, X *ini )

#define RESIZE_STRING( A, X )				\
  TYPE(A) resize_##A( TYPE(A) A, size_t new_len )

#define STRING_REF( A, X )			\
  X A##_ref( TYPE(A) A, long i )

#define STRING_SET( A, X )			\
  X A##_set( TYPE(A), long i, X x )

#endif
