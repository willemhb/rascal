#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// tags -----------------------------------------------------------------------
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

// value tags -----------------------------------------------------------------
#define SMALL     0x7ffd000000000000ul
#define INTEGER   0x7ffe000000000000ul
#define OBJECT    0x7fff000000000000ul
#define IMMEDIATE 0xfffd000000000000ul
#define ARITY     0xfffe000000000000ul
#define POINTER   0xffff000000000000ul

#define CHARACTER (SMALL|(3ul<<32))
#define BOOLEAN   (SMALL|(2ul<<32))
#define NIL       (IMMEDIATE|6ul)

#define TRUE      (BOOLEAN|1)
#define FALSE     (BOOLEAN|0)

typedef enum __attribute__((packed))
  {
    VAL_REAL=1,
    VAL_INT=2,
    VAL_BOOL=3,
    VAL_CHAR=4,
    VAL_ATOM=5,
    VAL_LIST=6,
    VAL_FUNCTION=7,
    VAL_STRING=9,
    VAL_VECTOR=10,
    VAL_MAP=11,
    VAL_SET=12,

    VAL_CLOSURE=13,
    VAL_MODULE=14,
 
    // internal types
    VAL_ENVIRONMENT=15,
    VAL_VAR=17,

    VAL_TEMPLATE=18,
    VAL_ARRAYLIST=19,
    VAL_BYTECODE=20,
    VAL_TABLE=21,
    VAL_ENTRY=22,

    // pad
    NUM_TYPES
  } ValueType;

typedef uintptr_t Value;
typedef double    Real;

// other internal types
typedef uint16_t   Instruction;
typedef Value    (*NativeFn)( Value *args, Arity nNargs );

typedef struct Obj        Obj;
typedef struct Atom       Atom;
typedef struct List       List;
typedef struct Function   Function;
typedef struct String     String;
typedef struct Vector     Vector;
typedef struct Map        Map;
typedef struct Set        Set;
typedef struct Closure    Closure;

typedef struct ArrayList  ArrayList;
typedef struct Table      Table;
typedef struct CodeBuffer CodeBuffer;
typedef struct ByteCode   ByteCode;

typedef union
  {
    Char     as_char;
    Bool     as_bool;
    Real     as_real;
    Value    as_val;
    Void    *as_ptr;
    Obj     *as_obj;

    struct
    {
      Int64 as_int : 48;
      Int64        : 16;
    };
  } ValueData;

typedef Hash (*HashFn)(Value val);
typedef Int  (*OrderFn)(Value a, Value b);

// globals --------------------------------------------------------------------
extern Obj    *Singletons[NUM_TYPES];
extern HashFn  HashDispatch[NUM_TYPES];
extern OrderFn OrderDispatch[NUM_TYPES];

// Generics & declarations ----------------------------------------------------
Hash      hashValue( Value k );
Int       orderValues( Value a, Value b );

ValueType valueTypeOf( Value val );
ValueType objectTypeOf( Obj *obj );

#define typeOf(val)				\
  _Generic((val),				\
	   Value:valueTypeOf,			\
	   Obj*:objectTypeOf)(val)

// utility macros & statics ---------------------------------------------------
#define isSmall(x)     (((x)&SMALL) == SMALL)
#define isInteger(x)   (((x)&INTEGER) == INTEGER)
#define isObj(x)       (((x)&OBJECT) == OBJECT)
#define isImmediate(x) (((x)&IMMEDIATE) == IMMEDIATE)
#define isArity(x)     (((x)&ARITY) == ARITY)
#define isPointer(x)   (((x)&POINTER) == POINTER)

#define isReal(x)      (((x)&QNAN) != QNAN)
#define isChar(x)      (((x)&CHARACTER) == CHARACTER)
#define isBool(x)      (((x)&BOOLEAN) == BOOLEAN)
#define isNil(x)       ((x) == NIL)

#define tagValue(x, t) ((((ValueData)(x)).as_val)|(t))
#define asValue(x)     (((ValueData)(x)).as_val)
#define asReal(x)      (((ValueData)(x)).as_real)
#define asInteger(x)   ((Int64)(((ValueData)(x)).as_int))
#define asArity(x)     ((Arity)((x)&~POINTER))
#define asBool(x)      ((x)==TRUE)
#define asChar(x)      ((Char)((x)&UINT32_MAX))

static inline Void *asPtr( Value x )
{
  if (isObj(x) || isPointer((x)))
    return (Void*)(x&~POINTER);

  if (isImmediate(x))
    return Singletons[x&255];    

  return NULL;
}

#endif
