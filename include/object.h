#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "common.h"
#include "value.h"

typedef Value (*NativeFn)(size_t argCount, Value* args);

// object array (used in various places)
#include "tpl/declare.h"

ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(ByteCode, uint16_t);
TABLE_TYPE(SymbolTable, symbolTable, char*, Symbol*);

struct Obj {
  struct Obj* next;        // live objects list
  uint64_t    hash;        // cached hash code
  Type        type;        // object type (duh)
  uint8_t     hashed;      // hash is valid or needs to be computed
  uint8_t     flags;       // misc, discretionary
  uint8_t     black;       // gc mark flag
  uint8_t     gray;        // gc trace flag
  uint8_t     data[];      // pointer to object's regular data
};

struct Symbol {
  Obj       obj;
  char*     name;
  uintptr_t idno; // unique identifier
};

struct Bits {
  Obj    obj;
  void*  data;
  size_t arity;
  size_t elSize;
};

struct List {
  Obj    obj;
  List*  tail;
  size_t arity;
  Value  head;
};

struct Tuple {
  Obj    obj;
  Value* data;
  size_t arity;
};

struct Map {
  Obj    obj;
  Node*  root;
  size_t arity;
};

struct Node {
  Obj    obj;
  Obj**  chidren;
  size_t bitmap;
  size_t offset;
};

struct Leaf {
  Obj   obj;
  Leaf* next; // handles collisions, mostly unused, simple but wastes space.
  Value key;
  Value bind;
};

struct Chunk {
  Obj      obj;
  ByteCode instructions;
  Values   constants;
};

struct Closure {
  Obj     obj;
  Chunk*  code;
  Objects upvalues;
};

struct UpValue {
  Obj      obj;
  UpValue* next;
  union {
    Value  bind;
    Value* location;
  };
  bool     open;
};

struct Native {
  Obj     obj;
  char*   name;
  Value (*callback)(Value* args);
};

struct Stream {
  Obj   obj;
  FILE* ios;
};

// miscellaneous utilities
uint64_t hashObject(void* ob);
bool     equalObjects(void* obX, void* obY);

// constructors
Symbol* newSymbol(char* name);
Symbol* getSymbol(char* token, size_t tokSize);

List*  newList(Value head, List* tail);
List*  newList1(Value head);
List*  newList2(Value arg1, Value arg2);
List*  newListN(size_t n, Value* args);

Tuple* newTuple(size_t arity, Value* slots);
Tuple* newPair(Value x, Value y);
Tuple* newTriple(Value x, Value y, Value z);

#endif
