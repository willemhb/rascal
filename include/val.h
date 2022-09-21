#ifndef rascal_val_h
#define rascal_val_h

#include "common.h"

#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define SMALL     0x7ffd000000000000ul
#define IMMEDIATE 0x7ffe000000000000ul
#define INTEGER   0x7fff000000000000ul
#define OBJECT    0xfffd000000000000ul
#define POINTER   0xfffe000000000000ul
#define ARITY     0xffff000000000000ul

typedef uintptr_t    val_t;
typedef double       real_t;
typedef intptr_t     int_t;
typedef char         char_t;
typedef bool         bool_t;
typedef struct obj_t obj_t;
typedef void        *ptr_t;

typedef uint32_t     val_type_t;

// value types
enum
  {
    NIL_TYPE,

    INT_TYPE,
    REAL_TYPE,
    BOOL_TYPE,
    CHAR_TYPE,

    ATOM_TYPE,
    CONS_TYPE,
    FUNC_TYPE,
    PORT_TYPE,
    STRING_TYPE,

    // internal types
    NATIVE_TYPE,
    CODE_TYPE,
    CLOSURE_TYPE,
    
    NUM_VALUE_TYPES
  };

#define NUM_TYPES_PAD 255

typedef union
{
  val_t   as_val;
  real_t  as_real;
  obj_t  *as_obj;
  ptr_t   as_ptr;

  struct
  {
    int_t as_int : 48;
    int_t        : 16;
  };
} val_data_t;



#endif
