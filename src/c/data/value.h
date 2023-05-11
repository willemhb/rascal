#ifndef data_value_h
#define data_value_h

#include "common.h"

// C types
// value types
typedef uword              value_t; // tagged lisp data
typedef double             real_t;
typedef uword              fixnum_t;
typedef bool               boolean_t;
typedef struct object      object_t;

// user object types
typedef struct symbol      symbol_t;
typedef struct list        list_t;
typedef struct binary      binary_t;
typedef struct dict        dict_t;
typedef struct vector      vector_t;
typedef struct function    function_t;
typedef struct chunk       chunk_t;
typedef struct closure     closure_t;

// type codes
typedef enum {
  // object types
  // user objects
  SYMBOL=1,
  LIST,
  BINARY,
  DICT,
  VECTOR,
  FUNCTION,
  CHUNK,
  CLOSURE,

  // value types
  OBJECT=CLOSURE,
  REAL,
  FIXNUM,
  BOOLEAN,
  UNIT,
  BOTTOM,
  TOP
} type_t;

#define NTYPES (TOP+1)

// tags
#define QNAN        0x7ff8000000000000ul
#define SIGN        0x8000000000000000ul
#define IMMTAG      0x7ffc000000000000ul
#define FIXTAG      0x7ffd000000000000ul
#define OBJTAG      0x7fff000000000000ul

#define WTMASK      0xffff000000000000ul
#define WVMASK      0x0000fffffffffffful
#define ITMASK      0xffffffff00000000ul
#define IVMASK      0x00000000fffffffful

// API & utilities
type_t value_type(value_t x);
type_t rascal_type(value_t x);

#endif
