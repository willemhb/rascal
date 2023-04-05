#ifndef value_h
#define value_h

#include "common.h"

// C types
// value types
typedef uword   value_t; // tagged lisp data
typedef double  real_t;
typedef uint64  fixnum_t;
typedef sint32  sint32_t;
typedef bool    boolean_t;
typedef char    ascii_t;
typedef void   *pointer_t;
typedef struct object object_t;

// user object types
typedef struct symbol   symbol_t;
typedef struct type     type_t;
typedef struct function function_t;
typedef struct port     port_t;
typedef struct list     list_t;
typedef struct string   string_t;

// internal object types
typedef struct variable variable_t;
typedef struct upvalue  upvalue_t;

typedef struct chunk    chunk_t;
typedef struct closure  closure_t;
typedef struct control  control_t;

// building block collection types
typedef struct table    table_t;
typedef struct alist    alist_t;
typedef struct buffer   buffer_t;

// type codes
typedef enum {
  SYMBOL,
  TYPE,
  FUNCTION,
  PORT,
  LIST,
  STRING,
  VARIABLE,
  UPVALUE,
  CHUNK,
  CLOSURE,
  CONTROL,
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
  NONE,
  ANY
} valtype_t;

#define NTYPES (ANY+1)

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

// utilities
valtype_t valtype(value_t vx);
objtype_t objtype(object_t* ox);
value_t   object(void* px);
value_t   fixnum(fixnum_t fx);
value_t   sint(sint32_t ix);
value_t   real(real_t rx);
value_t   pointer(pointer_t px);
value_t   boolean(boolean_t bx);

type_t*   val_type_of(value_t vx);
type_t*   obj_type_of(object_t* ox);

#define   type_of(x) generic2(type_of, x)

#endif
