#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// describe macros ------------------------------------------------------------
#define ArrayType(eltype, meta)			\
  {						\
    HEADER;					\
    eltype *data;				\
    uint len, meta;				\
  }

#define HashedArrayType(eltype, meta)		\
  {						\
    HEADER;					\
    eltype *data;				\
    uint len, meta;				\
    ulong hash;					\
  }

#define TupleType(eltype)			\
  {						\
    HEADER;					\
    eltype *data;				\
    value_t signature;				\
    ulong hash;					\
  }

// C types --------------------------------------------------------------------
typedef struct vector_t vector_t;
typedef struct tuple_t  tuple_t;
typedef struct string_t string_t;
typedef struct bytes_t  bytes_t;
typedef struct buffer_t buffer_t;
typedef struct record_t record_t;
typedef struct stack_t  stack_t;

struct vector_t ArrayType(value_t, cap);
struct string_t HashedArrayType(char, encoding);
struct bytes_t  ArrayType(uchar, Ctype);
struct buffer_t ArrayType(uchar, cap);
struct stack_t  ArrayType(value_t, cap);
struct record_t TupleType(value_t);
struct tuple_t  TupleType(value_t);

typedef struct vector_t environment_t;


// macros & statics -----------------------------------------------------------

#endif
