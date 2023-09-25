#ifndef rascal_value_h
#define rascal_value_h

#include <stdio.h>

#include "common.h"

// core rascal types
typedef uintptr_t      Value;   // standard tagged value representation (NaN boxed)
typedef double         Number;  // common number
typedef bool           Boolean; // boolean
typedef struct Obj     Obj;     // generic object
typedef struct Symbol  Symbol;  // interned symbol
typedef struct Bits    Bits;    // compact binary data
typedef struct List    List;    // immutable linked list
typedef struct Tuple   Tuple;   // immutable fixed-size collection

/*
typedef struct Map     Map;     // HAMT
typedef struct Node    Node;    // HAMT node
typedef struct Leaf    Leaf;    // HAMT entry
typedef struct Module  Module;  // user namespace
typedef struct Chunk   Chunk;   // compiled bytecode
typedef struct Closure Closure; // chunk + context
typedef struct UpValue UpValue; // local variable
typedef struct Native  Native;  // builtin function
typedef struct Stream  Stream;  // IO port */

// various & sundry enums
typedef enum {
  NOTYPE,
  NUMBER,
  BOOLEAN,
  UNIT,
  OBJECT,
  SYMBOL=OBJECT,
  BITS,
  LIST,
  TUPLE,
  /* 
  MAP,
  NODE,
  LEAF,
  MODULE,
  CHUNK,
  CLOSURE,
  UPVALUE,
  NATIVE,
  STREAM
  */
} Type;

#define NUM_TYPES (TUPLE+1)

// tags and such
#define NUL_TAG     0x7ffc000000000000UL
#define BOOL_TAG    0x7ffd000000000000UL
#define OBJ_TAG     0x7ffe000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL

#define TRUE_VAL    (BOOL_TAG | 1UL)
#define FALSE_VAL   (BOOL_TAG | 0UL)
#define NUL_VAL     (NUL_TAG  | 0UL)
#define NOTHING_VAL (NUL_TAG  | 1UL) // invalid value marker

#define TAG_BOOL(boolean) ((boolean) ? TRUE_VAL : FALSE_FAL)
#define TAG_NUM(number)   doubleToWord(number)
#define TAG_OBJ(pointer)  (((Value)(pointer)) | OBJ_TAG)

#define AS_BOOL(value)    ((value) == TRUE_VAL)
#define AS_NUM(value)     ((Number)wordToDouble(value))
#define AS_OBJ(value)     ((Obj*)((value) & VAL_MASK))
#define AS_SYMBOL(value)  ((Symbol*)((value) & VAL_MASK))
#define AS_BITS(value)    ((Bits*)((value) & VAL_MASK))
#define AS_LIST(value)    ((List*)((value) & VAL_MASK))
#define AS_TUPLE(value)   ((Tuple*)((value) & VAL_MASK))

/*
#define AS_MAP(value)     ((Map*)((value) & VAL_MASK))
#define AS_NODE(value)    ((Node*)((value) & VAL_MASK))
#define AS_LEAF(value)    ((Leaf*)((value) & VAL_MASK))
#define AS_CHUNK(value)   ((Chunk*)((value) & VAL_MASK))
#define AS_CLOSURE(value) ((Closure*)((value) & VAL_MASK))
#define AS_UPVALUE(value) ((UpValue*)((value) & VAL_MASK))
#define AS_NATIVE(value)  ((Native*)((value) & VAL_MASK))
#define AS_STREAM(value)  ((Stream*)((value) & VAL_MASK))
*/

#define IS_NUL(value)     ((value) == NUL_VAL)
#define IS_BOOL(value)    hasValueType(value, BOOLEAN)
#define IS_OBJ(value)     hasValueType(value, OBJECT)
#define IS_NUM(value)     (((value) & QNAN) != QNAN)
#define IS_SYMBOL(value)  hasRascalType(value, SYMBOL)
#define IS_BITS(value)    hasRascalType(value, BITS)
#define IS_LIST(value)    hasRascalType(value, LIST)
#define IS_TUPLE(value)   hasRascalType(value, TUPLE)

/*
#define IS_MAP(value)     hasRascalType(value, MAP)
#define IS_NODE(value)    hasRascalType(value, NODE)
#define IS_LEAF(value)    hasRascalType(value, LEAF)
#define IS_CHUNK(value)   hasRascalType(value, CHUNK)
#define IS_CLOSURE(value) hasRascalType(value, CLOSURE)
#define IS_UPVALUE(value) hasRascalType(value, UPVALUE)
#define IS_NATIVE(value)  hasRascalType(value, NATIVE)
#define IS_STREAM(value)  hasRascalType(value, STREAM)
*/

#include "tpl/declare.h"

ARRAY_TYPE(Values, Value);

void     printValues(FILE* ios, Values* values);
Type     valueType(Value value);
Type     rascalType(Value value);
Type     objectType(Obj* object);
size_t   sizeOfType(Type type);
char*    nameOfType(Type type);
bool     equalValues(Value x, Value y);
uint64_t hashValue(Value x);
void     printValue(FILE* ios, Value x, int indent);
uint64_t hashType(Type type);

static inline bool hasValueType(Value value, Type type) {
  return valueType(value) == type;
}

static inline bool hasRascalType(Value value, Type type) {
  return rascalType(value) == type;
}

#endif
