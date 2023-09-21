#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "common.h"
#include "value.h"

typedef Value (*NativeFn)(size_t argCount, Value* args);

// object array (used in various places)
#include "declare.h"

ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(Bytecode, uint16_t);

struct Obj {
  struct Obj* next;        // live objects list
  uintptr_t   hash   : 48; // cached object hash
  uintptr_t   type   :  6; // object's vm type (may be different than the type returned by `typeof`)
  uintptr_t   flags  :  5; // miscellaneous discretionary flags
  uintptr_t   hashed :  1; // indicates whether self->hash is valid
  uintptr_t   black  :  1; // gc marked flag
  uintptr_t   gray   :  1; // gc traced flag
  uint8_t     data[];      // pointer to object's regular data
};

struct Atom {
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
  Bytecode instructions;
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


#endif
