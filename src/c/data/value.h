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
typedef struct table       table_t;
typedef struct vector      vector_t;
typedef struct function    function_t;
typedef struct native      native_t;
typedef struct namespace   namespace_t;
typedef struct environment environment_t;
typedef struct chunk       chunk_t;
typedef struct closure     closure_t;
typedef struct variable    variable_t;
typedef struct upvalue     upvalue_t;

// type codes
typedef enum {
  // object types
  SYMBOL=1,
  LIST,
  BINARY,
  TABLE,
  VECTOR,
  FUNCTION,
  NATIVE,
  NAMESPACE,
  ENVIRONMENT,
  CHUNK,
  CLOSURE,
  VARIABLE,
  UPVALUE,

  // value types
  OBJECT=UPVALUE,
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

// APIs & utilities
#define   rl_typeof(x)    generic2(typeof, x)
#define   rl_sizeof(x)    generic2(sizeof, x)
#define   rl_isa(x, t)    generic2(isa, x, t)
#define   rl_mark(x)      generic2(mark, x)
#define   rl_asa(x, m, t) ((t)(((uword)(x)) & (m)))

type_t valtype(value_t vx);
type_t val_typeof(value_t vx);
usize val_sizeof(value_t vx);
bool val_isa(value_t vx, type_t tx);
void val_mark(value_t vx);
void mark_values(usize n, value_t* vals);

// defined in object.c|h
extern type_t objtype(void* ox);
extern type_t obj_typeof(void* ox);
extern usize obj_sizeof(void* ox);
extern bool obj_isa(void* ox, type_t tx);
extern void obj_mark(void* ox);

#endif
