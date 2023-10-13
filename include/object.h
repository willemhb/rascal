#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"

// generics
#include "tpl/declare.h"

ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(ByteCode, uint16_t);
TABLE_TYPE(SymbolTable, symbolTable, char*, Symbol*);
TABLE_TYPE(NameSpace, nameSpace, Symbol*, Binding*);

struct Obj {
  Obj*     next;   // live objects list
  Obj*     annot;  // object metadata
  uint64_t hash;   // cached hash code
  TypeCode type;   // VM type of this object (doesn't distinguish struct or record types)
  uint8_t  flags;  // miscellaneous flags
  uint8_t  hashed; // indicates whether self->hash is valid
  uint8_t  gray;   // gc mark flag
  uint8_t  black;  // gc trace flag
  uint8_t  data[]; // pointer to object's regular data
};

typedef size_t (*CompileFn)(Vm* vm, List* form);

// user types
struct Symbol {
  Obj       obj;
  char*     name;
  uintptr_t idno;    // unique identifier
  CompileFn special; // special form associated with this symbol
};

struct Function {
  Obj          obj;
  Symbol*      name;
  Obj*         handler; // probably a method table
};

struct Type {
  Obj       obj;
  Symbol*   name;
  Type*     parent;        // abstract base type
  Type*     left, * right; // union constituents stored as invasive tree
  Function* ctor;          // constructor for values of this type
  TypeCode  code;          // VM type code (if applicable)
  uint32_t  idno;          // unique counter
};

struct Binding {
  Obj     obj;
  Symbol* name;            // plain name of the binding (unqualified)
  Scope*  scope;           // scope in which the name is defined
  size_t  offset;          // offset within corresponding environment
  Value   value;           // value associated with the binding (if the value is stored directly)
};

struct Stream {
  Obj     obj;
  FILE*   ios;
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

struct Vector {
  Obj      obj;
  VecNode* root;   // if arity is greater than 64, remaining elements are stored here
  size_t   arity;  // total number of elements in the vector
  Type*    sType;  // struct type (NULL if this is a plain Vector)
  Value    tail[]; // last 64 elements are stored immediately
};

struct Map {
  Obj      obj;
  MapNode* root;  // key/value pairs stored here
  size_t   arity; // total number of key/value pairs
  Type*    rType; // record type (NULL if this is a plain Map)
};

struct Big {
  Obj      obj;
  uint8_t* digits;
  uint32_t arity;
  int      sign;
};

TABLE_TYPE(MethodCache, methodCache, List*, Obj*);

struct MethodTable {
  Obj         obj;
  MethodCache cache;
  Obj*        zeroArityMethod;
  List*       fixedArityMethods;
  List*       variadicMethods;
};

typedef Value (*NativeFn)(size_t nArgs, Value* args);

struct Native {
  Obj      obj;
  NativeFn callBack;
};

struct Chunk {
  Obj      obj;
  Obj*     name;  // readable name (might be string or symbol)
  Values   vals;  // constant store
  ByteCode code;  // instructions
};

struct Closure {
  Obj          obj;
  Chunk*       code;
  Environment* envt;
};

struct Scope {
  Obj       obj;
  Scope*    parent;
  NameSpace locals;
  bool      toplevel;
  bool      private;
};

struct Environment {
  Obj          obj;
  Environment* parent;
  Scope*       scope;
  Values       vals;
};

// global sigletons
extern List   emptyList;
extern Vector emptyVector;
extern Map    emptyMap;

#define EMPTY_LIST()   TAG_OBJ(&emptyList)
#define EMPTY_VECTOR() TAG_OBJ(&emptyVector)
#define EMPTY_MAP()    TAG_OBJ(&emptyMap)

// miscellaneous utilities
uint64_t hashObject(void* ob);
bool     equalObjects(void* obx, void* oby);
void     freeObject(void* ob);

// constructors
Symbol* newSymbol(char* name);
Symbol* getSymbol(char* token);

Chunk* newChunk(Symbol* name);

Bits*   newBits(void* data, size_t count, size_t elSize, Encoding encoding);
Bits*   newString(char* data, size_t count);

List*  newList(Value head, List* tail);
List*  newList1(Value head);
List*  newList2(Value arg1, Value arg2);
List*  newListN(size_t n, Value* args);
Value  mkListN(size_t n, Value* args);

#endif
