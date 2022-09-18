#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// tags -----------------------------------------------------------------------
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

// value tags -----------------------------------------------------------------
#define IMMEDIATE 0x7ffc000000000000ul
#define ATOM      0x7ffd000000000000ul
#define LIST      0x7ffe000000000000ul
#define FUNCTION  0x7fff000000000000ul
#define BINARY    0xfffc000000000000ul
#define VECTOR    0xfffd000000000000ul
#define MAP       0xfffe000000000000ul
#define CLOSURE   0xffff000000000000ul

#define BOOLEAN   (IMMEDIATE|(1ul<<32))
#define CHARACTER (IMMEDIATE|(2ul<<32))
#define TRUE      (BOOLEAN|1)
#define FALSE     (BOOLEAN|0)

typedef enum __attribute__((packed))
  {
    VAL_REAL,
    VAL_BOOL,
    VAL_CHAR,
    VAL_ATOM,
    VAL_LIST,
    VAL_FUNCTION,
    VAL_BINARY,
    VAL_VECTOR,
    VAL_MAP,
    VAL_CLOSURE,

    // internal types
    // list types
    VAL_UPVALUE,
 
    // binary types
    VAL_NATIVE,
    VAL_ARRAYLIST,
    VAL_BYTECODE,
    VAL_TABLE,
    VAL_ENTRY,

    // pad
    NUM_TYPES
  } ValueType;

typedef uintptr_t Value;
typedef double    Real;

// other internal types
typedef uint16_t   Instruction;
typedef Value    (*NativeFn)( Value *args, Arity nNargs );

typedef struct Obj      Obj;
typedef struct Atom     Atom;
typedef struct List     List;
typedef struct Function Function;
typedef struct Binary   Binary;
typedef struct Vector   Vector;
typedef struct Map      Map;
typedef struct Closure  Closure;

typedef struct ArrayList ArrayList;
typedef struct Table     Table;
typedef struct ByteCode  ByteCode;
typedef struct UpValue   UpValue;
typedef struct Native    Native;

typedef union
  {
    Char     as_char;
    Bool     as_bool;
    Real     as_real;
    Value    as_val;
    Void    *as_ptr;
  } ValueData;

// forward declarations -------------------------------------------------------
Hash hashValue( Value k );

// utility macros & statics ---------------------------------------------------
#define tagPtr(p, t)   (((Value)(p))|(t))
#define isImmediate(x) (((x)&IMMEDIATE) == IMMEDIATE)
#define isReal(x)      (((x)&QNAN) != QNAN)
#define isChar(x)      (((x)&CHARACTER) == CHARACTER)
#define isBool(x)      (((x)&BOOLEAN) == BOOLEAN)
#define isAtom(x)      (((x)&ATOM) == ATOM)
#define isList(x)      (((x)&LIST) == LIST)
#define isFunction(x)  (((x)&FUNCTION) == FUNCTION)
#define isBinary(x)    (((x)&BINARY) == BINARY)
#define isVector(x)    (((x)&VECTOR) == VECTOR)
#define isMap(x)       (((x)&MAP) == MAP)
#define isClosure(x)   (((x)&CLOSURE) == CLOSURE)

#define asValue(x, t) ((((ValueData)(x)).as_val)|(t))
#define asReal(x)     (((ValueData)(x)).as_real)
#define asArity(x)    ((Arity)((x)&UINT32_MAX))
#define asBool(x)     ((x)==TRUE)
#define asChar(x)     ((Char)((x)&UINT32_MAX))
#define asPtr(x)      ((Void*)((x)&~CLOSURE))

#define asObjType(obType, x)				\
  ((obType*)_Generic((x),				\
		    Value:(((Value)(x))&~CLOSURE),	\
		     default:((obType*)(x))))

#define asObj(x)      asObjType(Obj, x)
#define asAtom(x)     asObjType(Atom, x)
#define asList(x)     asObjType(List, x)
#define asFunction(x) asObjType(Function, x)
#define asBinary(x)   asObjType(Binary, x)
#define asVector(x)   asObjType(Vector, x)
#define asMap(x)      asObjType(Map, x)
#define asClosure(x)  asObjType(Closure, x)

#endif
