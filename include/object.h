#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "common.h"
#include "value.h"

// generics
#include "tpl/declare.h"

ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(ByteCode, uint16_t);
TABLE_TYPE(SymbolTable, symbolTable, char*, Symbol*);
TABLE_TYPE(NameSpace, nameSpace, Symbol*, size_t);

struct Obj {
  Obj*     next;   // live objects list
  uint64_t hash;   // cached hash code
  Type     type;   // VM type of this object (doesn't distinguish struct or record types)
  uint8_t  flags;  // miscellaneous flags
  uint8_t  hashed; // indicates whether self->hash is valid
  uint8_t  gray;   // gc mark flag
  uint8_t  black;  // gc trace flag
  uint8_t  data[]; // pointer to object's regular data
};

typedef size_t (*CompileFn)(Compiler* state, List* form);

struct Symbol {
  Obj       obj;
  char*     name;
  uintptr_t idno;    // unique identifier
  CompileFn special; // special form associated with this symbol
};

typedef Value  (*NativeFn)(size_t nArgs, Value* args);

struct Native {
  Obj      obj;
  NativeFn callBack;
};

struct Chunk {
  Obj      obj;
  Values   vals;
  ByteCode code;
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

// global sigletons
extern List  emptyList;

#define EMPTY_LIST()  TAG_OBJ(&emptyList)

// miscellaneous utilities
uint64_t hashObject(void* ob);
bool     equalObjects(void* obx, void* oby);
void     freeObject(void* ob);

// constructors
Symbol* newSymbol(char* name);
Symbol* getSymbol(char* token);

Chunk* newChunk(void);

Bits*   newBits(void* data, size_t count, size_t elSize, Encoding encoding);
Bits*   newString(char* data, size_t count);

List*  newList(Value head, List* tail);
List*  newList1(Value head);
List*  newList2(Value arg1, Value arg2);
List*  newListN(size_t n, Value* args);
Value  mkListN(size_t n, Value* args);

#endif
