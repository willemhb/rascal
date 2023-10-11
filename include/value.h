#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// core rascal types
typedef uintptr_t       Value;    // standard tagged value representation (NaN boxed)
typedef double          Number;   // common number
typedef bool            Boolean;  // boolean
typedef struct Obj      Obj;      // generic object
typedef struct Symbol   Symbol;   // interned symbol
typedef struct Native   Native;   // native function or special form
typedef struct Chunk    Chunk;    // compiled code
typedef struct Bits     Bits;     // compact binary data
typedef struct List     List;     // immutable linked list

// various & sundry enums
typedef enum {
  NOTYPE,
  NUMBER,
  BOOLEAN,
  UNIT,
  OBJECT,
  SYMBOL=OBJECT,
  NATIVE,
  CHUNK,
  BITS,
  LIST,
} Type;

#define NUM_TYPES (LIST+1)

// tags and such
#define NUL_TAG     0x7ffc000000000000UL
#define BOOL_TAG    0x7ffd000000000000UL
#define OBJ_TAG     0x7ffe000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL

#define TRUE        (BOOL_TAG | 1UL)
#define FALSE       (BOOL_TAG | 0UL)
#define NUL         (NUL_TAG  | 0UL)
#define NOTHING     (NUL_TAG  | 1UL) // invalid value marker

#define TAG_BOOL(boolean) ((boolean) ? TRUE : FALSE)
#define TAG_NUM(number)   doubleToWord(number)
#define TAG_OBJ(pointer)  (((Value)(pointer)) | OBJ_TAG)

#define AS_BOOL(value)    ((value) == TRUE)
#define AS_NUM(value)     ((Number)wordToDouble(value))
#define AS_OBJ(value)     ((Obj*)((value) & VAL_MASK))
#define AS_SYMBOL(value)  ((Symbol*)((value) & VAL_MASK))
#define AS_NATIVE(value)  ((Native*)((value) & VAL_MASK))
#define AS_CHUNK(value)   ((Chunk*)((value) & VAL_MASK))
#define AS_BITS(value)    ((Bits*)((value) & VAL_MASK))
#define AS_LIST(value)    ((List*)((value) & VAL_MASK))

#define IS_NUL(value)     ((value) == NUL)
#define IS_BOOL(value)    hasValueType(value, BOOLEAN)
#define IS_OBJ(value)     hasValueType(value, OBJECT)
#define IS_NUM(value)     (((value) & QNAN) != QNAN)
#define IS_SYMBOL(value)  hasRascalType(value, SYMBOL)
#define IS_NATIVE(value)  hasRascalType(value, NATIVE)
#define IS_CHUNK(value)   hasRascalType(value, CHUNK)
#define IS_BITS(value)    hasRascalType(value, BITS)
#define IS_LIST(value)    hasRascalType(value, LIST)

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
void     printValue(FILE* ios, Value x);
void     printLine(FILE* ios, Value x);
uint64_t hashType(Type type);

static inline bool hasValueType(Value value, Type type) {
  return valueType(value) == type;
}

static inline bool hasRascalType(Value value, Type type) {
  return rascalType(value) == type;
}

#endif
