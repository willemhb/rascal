#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// core rascal types
typedef uintptr_t      Value;   // standard tagged value representation (NaN boxed)
typedef double         Number;  // common number
typedef bool           Boolean; // boolean
typedef struct Obj     Obj;     // generic object
typedef struct Atom    Atom;    // interned symbol
typedef struct Bits    Bits;    // compact binary data
typedef struct List    List;    // immutable linked list
typedef struct Tuple   Tuple;   // immutable fixed-size collection
typedef struct Map     Map;     // HAMT
typedef struct Node    Node;    // HAMT node
typedef struct Leaf    Leaf;    // HAMT entry
typedef struct Chunk   Chunk;   // compiled bytecode
typedef struct Closure Closure; // chunk + context
typedef struct UpValue UpValue; // local variable
typedef struct Native  Native;  // builtin function
typedef struct Stream  Stream;  // IO port

// various & sundry enums
typedef enum {
  NOTYPE,
  NUMBER,
  BOOLEAN,
  UNIT,
  OBJECT,
  ATOM=OBJECT,
  BITS,
  LIST,
  TUPLE,
  MAP,
  NODE,
  LEAF,
  CHUNK,
  CLOSURE,
  UPVALUE,
  NATIVE,
  STREAM
} Type;

// tags and such
#define NIL_TAG     0x7ffc000000000000UL
#define BOOL_TAG    0x7ffd000000000000UL
#define OBJ_TAG     0x7ffe000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL

#define TRUE_VAL    (BOOL_TAG | 1UL)
#define FALSE_VAL   (BOOL_TAG | 0UL)
#define NIL_VAL     (NIL_TAG  | 0UL)
#define NOTHING_VAL (NIL_TAG  | 1UL) // invalid value marker

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_OBJ(value)  ((Obj*)((value) & VAL_MASK))
#define AS_NUM(value)  ((Number)wordToDouble(value))

#define IS_NIL(value)     ((value) == NIL_VAL)
#define IS_BOOL(value)    hasValueType(value, BOOLEAN)
#define IS_OBJ(value)     hasValueType(value, OBJECT)
#define IS_NUM(value)     (((value) & QNAN) != QNAN)
#define IS_ATOM(value)    hasRascalType(value, ATOM)
#define IS_BITS(value)    hasRascalType(value, BITS)
#define IS_LIST(value)    hasRascalType(value, LIST)
#define IS_TUPLE(value)   hasRascalType(value, TUPLE)
#define IS_MAP(value)     hasRascalType(value, MAP)
#define IS_NODE(value)    hasRascalType(value, NODE)
#define IS_LEAF(value)    hasRascalType(value, LEAF)
#define IS_CLOSURE(value) hasRascalType(value, CLOSURE)

#include "declare.h"

ARRAY_TYPE(Values, Value);

Type     valueType(Value value);
Type     rascalType(Value value);
Type     objectType(Obj* object);
size_t   sizeOfType(Type type);
char*    nameOfType(Type type);
bool     equalValues(Value x, Value y);
uint64_t hashValue(Value x);

static inline bool hasValueType(Value value, Type type) {
  return valueType(value) == type;
}

static inline bool hasRascalType(Value value, Type type) {
  return rascalType(value) == type;
}

#endif
