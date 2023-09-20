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
#define QNAN     0x7ff8000000000000UL
#define SIGN     0x8000000000000000UL

#define NILTAG   0x7ffc000000000000UL
#define BOOLTAG  0x7ffd000000000000UL
#define OBJTAG   0x7ffe000000000000UL

#define TAGMASK  0xffff000000000000UL
#define VALMASK  0x0000ffffffffffffUL

#define AS_OBJ(value) ((Obj*)((value) & VALMASK))

#include "declare.h"

ARRAY_TYPE(ValuesArray, Value);

Type valueType(Value value);
Type rascalType(Value value);
Type objectType(Obj* object);

#endif
