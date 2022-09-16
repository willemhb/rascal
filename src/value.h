#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// tags -----------------------------------------------------------------------
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define BOOLEAN   0x7ffd000000000000ul
#define CHARACTER 0x7ffe000000000000ul
#define POINTER   0x7fff000000000000ul

#define CSTRING   0xfffd000000000000ul
#define OBJECT    0xfffe000000000000ul
#define HEADER    0xffff000000000000ul

#define GRAY      0x0000000000000004ul
#define BLACK     0x0000000000000002ul
#define ALLOCATED 0x0000000000000001ul

#define FALSE (BOOLEAN|0)
#define TRUE  (BOOLEAN|1)
#define NIL   (OBJECT)

// typedefs -------------------------------------------------------------------
typedef enum
  {
    VAL_REAL,
    VAL_BOOL,
    VAL_CHAR,
    VAL_POINTER,
    VAL_CSTRING,
    VAL_OBJECT
  } ValueType;

typedef uintptr_t Value;
typedef char      Char;
typedef bool      Bool;
typedef double    Real;
typedef void*     Pointer;
typedef char*     Cstring;

// other internal types
typedef uint16_t  Instruction;
typedef Value   (*NativeFn)( Value *args, int nNargs );

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
bool      sameValues( Value x, Value y );
bool      equalValues( Value x, Value y );
int       orderValues( Value x, Value y );
void      printValue( Value x );
hash_t    hashValue( Value x );

// utility macros & statics ---------------------------------------------------
#define IS_REAL(x)    (((x)&QNAN) != QNAN)
#define IS_NIL(x)     ((x)==NIL)
#define IS_CHAR(x)    (((x)&CHARACTER) == CHARACTER)
#define IS_BOOL(x)    (((x)&BOOLEAN) == BOOLEAN)
#define IS_PTR(x)     (((x)&POINTER) == POINTER)
#define IS_CSTR(x)    (((x)&CSTRING) == CSTRING)
#define IS_HEDR(x)    (((x)&HEADER) == HEADER)
#define IS_OBJ(x)     (isObject(x))

#define AS_VALUE(x, t) ((((ValueData)(x)).as_val)|(t))
#define AS_REAL(x)     (((ValueData)(x)).as_real)
#define AS_BOOL(x)     ((x)==TRUE)
#define AS_CHAR(x)     ((Char)((x)&UINT32_MAX))
#define AS_PTR(x)      ((Pointer)((x)&~HEADER))
#define AS_CSTR(x)     ((Cstring)((x)&~HEADER))
#define AS_OBJ(x)      ((Obj*)((x)&~HEADER))
#define AS_NEXTOBJ(x)  ((Obj*)((x)&~HEADER&~7ul))

static inline bool isObject( Value x )
{
  return (x&OBJECT) == OBJECT && x != NIL;
}

#endif
