#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

typedef uintptr_t    Value;  // standard tagged value representation (NaN boxed)
typedef double       Number; // common number
typedef bool         Bool;   // boolean
typedef struct Obj   Obj;    // generic object
typedef struct Atom  Atom;   // interned symbol
typedef struct Bits  Bits;   // compact binary data
typedef struct List  List;   // immutable linked list
typedef struct Tuple Tuple;  // immutable fixed-size collection
typedef struct Map   Map;    // HAMT
typedef struct Node  Node;   // HAMT node
typedef struct Leaf  Leaf;   // HAMT entry

#endif
