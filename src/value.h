#ifndef value_h
#define value_h

#include "common.h"

// C types
// value types
typedef uword              value_t; // tagged lisp data
typedef double             real_t;
typedef uint64             fixnum_t;
typedef sint32             sint32_t;
typedef bool               boolean_t;
typedef void*              pointer_t;
typedef struct object      object_t;

// user object types
typedef struct symbol      symbol_t;
typedef struct list        list_t;
typedef struct chunk       chunk_t;
typedef struct lambda      lambda_t;
typedef struct native      native_t;
typedef struct primitive   primitive_t;
typedef struct closure     closure_t;
typedef struct namespace   namespace_t;
typedef struct environment environment_t;
typedef struct variable    variable_t;
typedef struct upvalue     upvalue_t;
typedef struct table       table_t;
typedef struct alist       alist_t;
typedef struct buffer      buffer_t;

// type codes
typedef enum {
  SYMBOL,
  LIST,
  CHUNK,
  LAMBDA,
  NATIVE,
  PRIMITIVE,
  CLOSURE,
  NAMESPACE,
  ENVIRONMENT,
  VARIABLE,
  UPVALUE,
  TABLE,
  ALIST,
  BUFFER
} objtype_t;

typedef enum {
  OBJECT=BUFFER,
  REAL,
  FIXNUM,
  SINT32,
  BOOLEAN,
  POINTER,
  UNIT,
  BOTTOM,
  TOP
} valtype_t;

#define NTYPES (TOP+1)

// tags
#define QNAN        0x7ff8000000000000ul
#define SIGN        0x8000000000000000ul
#define IMMTAG      0x7ffc000000000000ul
#define FIXTAG      0x7ffd000000000000ul
#define PTRTAG      0x7ffe000000000000ul
#define OBJTAG      0x7fff000000000000ul

#define SINT32TAG  (IMMTAG | (((uword)SINT32)  << 32))
#define BOOLEANTAG (IMMTAG | (((uword)BOOLEAN) << 32))
#define UNITTAG    (IMMTAG | (((uword)UNIT)    << 32))

#define WTMASK      0xffff000000000000ul
#define WVMASK      0x0000fffffffffffful
#define ITMASK      0xffffffff00000000ul
#define IVMASK      0x00000000fffffffful

#define NUL        (UNITTAG    | 0)
#define TRUE       (BOOLEANTAG | 1)
#define FALSE      (BOOLEANTAG | 0)

#define NOTFOUND   (UNITTAG    | 1)
#define UNDEFINED  (UNITTAG    | 3)
#define UNBOUND    (UNITTAG    | 5)

// APIs & utilities
valtype_t valtype(value_t vx);
objtype_t objtype(object_t* ox);
value_t   object(void* px);
value_t   real(real_t rx);
value_t   fixnum(fixnum_t fx);
value_t   sint(sint32_t ix);
value_t   pointer(pointer_t px);
value_t   boolean(boolean_t bx);

#define   rl_typeof(x) generic2(type_of, x)
#define   rl_isa(x, t)  generic2(isa, x, t)

uhash     rl_hash(value_t x);
bool      rl_equal(value_t x, value_t y);

#define   as_object(x)  ((object_t*)((x) & WVMASK))
#define   as_real(x)    (((ieee64_t)(x)).dbl)
#define   as_fixnum(x)  ((x) & WVMASK)
#define   as_sint(x)    ((sint32_t)(x))
#define   as_pointer(x) ((pointer_t)(x))
#define   as_boolean(x) ((x) == TRUE)

#define   is_object(x)  (((x) & WTMASK) == OBJTAG)
#define   is_real(x)    (((x) & QNAN) != QNAN)
#define   is_fixnum(x)  (((x) & WTMASK) == FIXTAG)
#define   is_sint(x)    (((x) & ITMASK) == SINT32TAG)
#define   is_pointer(x) (((x) & WTMASK) == PTRTAG)
#define   is_boolean(x) (((x) & ITMASK) == BOOLEANTAG)

#endif
