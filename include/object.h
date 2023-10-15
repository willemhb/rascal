#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"

// generics
#include "tpl/declare.h"

ARRAY_TYPE(Values, Value);
ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(ByteCode, uint16_t);
TABLE_TYPE(NsMap, nsMap, Symbol*, Binding*);
TABLE_TYPE(MethodCache, methodCache, List*, Obj*);

struct Obj {
  Obj*     next;         // live objects list
  Map*     annot;        // object metadata
  Type*    type;         // type object
  uint64_t hash    : 48; // compressed hash
  uint64_t flags   : 11; // miscellaneous discretionary flags
  uint64_t hashed  :  1; // indicates whether self->hash is valid
  uint64_t noSweep :  1; // indicates that an object wasn't allocated with malloc (don't call deallocate)
  uint64_t noFree  :  1; // indicates that an object's data is allocated statically (don't call free)
  uint64_t gray    :  1; // gc mark flag
  uint8_t  black   :  1; // gc trace flag
  uint8_t  data[];
};

// user types
struct Symbol {
  Obj       obj;
  char*     name;
  uintptr_t idno;    // unique identifier
  CompileFn special; // special form associated with this symbol
};

struct Function {
  Obj          obj;
  Symbol*      name;       // the name of this function (or `fun` if anonymous).
  MethodTable* methods;    // handles method resolution
  Obj*         singleton;  // short-circuit dispatch if this is non-null
};

struct Vtable {
  size_t    valSize;         // base value size  (0 - 8 bytes)
  size_t    objSize;         // base object size (32+ bytes)
  uintptr_t tag;             // tag used for values of given type
  SizeFn    sizeOf;
  AllocFn   alloc;
  InitFn    init;
  CloneFn   clone;
  TraceFn   trace;
  FreeFn    free;
  PrintFn   print;
  HashFn    hash;
  EgalFn    equal;
  OrdFn     order;
};

struct Type {
  Obj       obj;
  Symbol*   name;
  Type*     parent;        // abstract base type
  Type*     left, * right; // union constituents stored as invasive tree
  Function* ctor;          // constructor for values of this type
  Vtable*   vTable;        // runtime and internal methods for types with concrete values
  uintptr_t idno;          // unique identifier for this type (similar to symbol idno)
  Kind      kind;          // how this type is constituted
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

struct Tuple {
  Obj    obj;
  size_t arity;
  Value  slots[];
};

struct List {
  Obj    obj;
  List*  tail;
  size_t arity;
  Value  head;
};

struct Vector {
  Obj      obj;
  VecNode* root;      // if arity is greater than 64, remaining elements are stored here
  size_t   arity;     // total number of elements in the vector
  Value*   tail;      // the last 64 elements are stored here
  size_t   shift;
};

struct Map {
  Obj      obj;
  MapNode* root;  // key/value pairs stored in underlying structure
  size_t   arity; // total number of key/value pairs
};

struct Big {
  Obj     obj;
  /* TODO: change to arbitrary precision. */
  int64_t value;
};

struct MethodTable {
  Obj         obj;
  MethodCache cache;
  Obj*        zeroArityMethod;
  List*       fixedArityMethods;
  List*       variadicMethods;
};

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

/* a single stack frame. */
struct Frame {
  Closure*  code;
  uint16_t* ip;
  size_t    bp;
};

struct Control {
  Obj      obj;
  Frame    frame;
  Value*   stackCopy;
  Frame*   framesCopy;
  size_t   nStack, nFrames;
};

struct Scope {
  Obj       obj;
  Scope*    parent;
  NsMap     locals;
  NsMap     upVals;
};

struct NameSpace {
  Obj    obj;
  NsMap* globals;
  NsMap* private;
  Scope* scope;
};

struct Environment {
  Obj          obj;
  Environment* parent;
  NameSpace*   ns;
  Objects      upvals;
};

struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

// node types
#define BRANCH_FACTOR 0x40ul
#define INDEX_MASK    0x3ful
#define LEVEL_SHIFT   0x06ul
#define MAX_SHIFT     0x30ul
#define MAX_LEVEL     0x08ul

struct VecNode {
  Obj      obj;
  Obj**    children;
  uint32_t shift;
  uint16_t count;
  uint16_t capacity;
};

struct VecLeaf {
  Obj   obj;
  Value slots[BRANCH_FACTOR];
};

struct MapNode {
  Obj      obj;
  Obj**    children;
  uint32_t shift;
  uint16_t count;
  uint16_t capacity;
  uint64_t bitmap;
};

struct MapLeaf {
  Obj      obj;
  MapLeaf* next; // used for collision resolution.
  Value    key;
  Value    val;
};

// global sigletons
extern List   emptyList;
extern Vector emptyVector;
extern Map    emptyMap;

// miscellaneous utilities
int getFl(void* p, int f, int m);
int setFl(void* p, int f, int m);
int delFl(void* p, int f);

void* clone(void* obj, int fl);
void* newObj(Type* type, int fl, size_t n, void* data);
void* allocObj(Type* type, int fl, size_t n);
void  initObj(void* obj, Type* type, int fl, size_t n, void* data);

// constructors
Symbol*   newSymbol(char* name, bool interned);
Function* newFunction(Symbol* name, Obj* ini, bool generic, bool macro);
Chunk*    newChunk(Symbol* name);
Bits*     newBits(void* data, size_t count, size_t elSize, Encoding encoding);
List*     newList(Value head, List* tail);
Vector*   newVector(size_t n, Value* vs);
Map*      newMap(size_t n, Value* kvs);

VecNode*  newVecNode();
VecLeaf*  newVecLeaf(Value* tail);

// convenience constructors
Symbol*   symbol(char* token);
Symbol*   gensym(char* name);

// collection interfaces
Value     mapGet(Map* m, Value k);
Map*      mapSet(Map* m, Value k, Value v);
Map*      mapAdd(Map* m, Value k, Value v);
Map*      mapDel(Map* m, Value k);

Value     vecGet(Vector* v, size_t i);
Vector*   vecAdd(Vector* v, Value x);
Vector*   vecSet(Vector* v, size_t i, Value x);
Vector*   vecDel(Vector* v);

Value     listGet(List* l, size_t n);
List*     listAdd(List* l, Value x);
List*     listSet(List* l, size_t n, Value x);
List*     listDel(List* l, size_t n);

#endif
