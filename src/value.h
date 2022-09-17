#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// tags -----------------------------------------------------------------------
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

// value tags -----------------------------------------------------------------
#define IMMEDIATE 0x7ffc000000000000ul
#define CHARACTER 0x7ffd000000000000ul
#define INTEGER   0x7ffe000000000000ul
#define OBJECT    0x7fff000000000000ul

// internal tags --------------------------------------------------------------
#define ARITY     0xfffc000000000000ul
#define POINTER   0xfffd000000000000ul
#define CSTRING   0xfffe000000000000ul
#define HEADER    0xffff000000000000ul

#define BOOLEAN  (IMMEDIATE|2)
#define NIL      (IMMEDIATE|1)
#define FALSE    (BOOLEAN|0)
#define TRUE     (BOOLEAN|1)

// typedefs -------------------------------------------------------------------
typedef enum
  {
    VAL_NIL=1,
    VAL_BOOL=2,
    VAL_CHAR=3,
    VAL_REAL=4,
    VAL_INT=5,
    VAL_OBJ=6,

    TAG_ARITY=7, // 48-bit unsigned integer (used internally for hashes)
    TAG_PTR=8,   // 
    TAG_CSTR=9,  // 
    TAG_HEDR=10, // 
  } ValueType;

typedef uintptr_t Value;
typedef double    Real;
typedef Void*     Pointer;
typedef Char*     Cstring;

// other internal types
typedef uint16_t  Instruction;
typedef Value   (*NativeFn)( Value *args, Int nNargs );

typedef struct Obj Obj;

typedef union
  {
    Char     as_char;
    Bool     as_bool;
    Real     as_real;
    Value    as_val;
    Obj     *as_obj;
    Pointer  as_ptr;
    Cstring  as_cstr;
  } ValueData;

// forward declarations -------------------------------------------------------
ValueType valueType( Value x );
Bool      sameValues( Value x, Value y );
Bool      equalValues( Value x, Value y );
Int       orderValues( Value x, Value y );
Void      printValue( Value x );
Hash      hashValue( Value x );

// utility macros & statics ---------------------------------------------------
#define IS_REAL(x)    (((x)&QNAN) != QNAN)
#define IS_NIL(x)     ((x)==NIL)
#define IS_CHAR(x)    (((x)&CHARACTER) == CHARACTER)
#define IS_BOOL(x)    (((x)&BOOLEAN) == BOOLEAN)
#define IS_PTR(x)     (((x)&POINTER) == POINTER)
#define IS_CSTR(x)    (((x)&CSTRING) == CSTRING)
#define IS_HEDR(x)    (((x)&HEADER) == HEADER)
#define IS_OBJ(x)     (((x)&OBJECT) == OBJECT)

#define AS_VALUE(x, t) ((((ValueData)(x)).as_val)|(t))
#define AS_REAL(x)     (((ValueData)(x)).as_real)
#define AS_BOOL(x)     ((x)==TRUE)
#define AS_CHAR(x)     ((Char)((x)&UINT32_MAX))
#define AS_PTR(x)      ((Pointer)((x)&~HEADER))
#define AS_CSTR(x)     ((Cstring)((x)&~HEADER))
#define AS_OBJ(x)      ((Obj*)((x)&~HEADER))

#endif
