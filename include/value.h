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

#endif
