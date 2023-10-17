#ifndef rascal_collection_h
#define rascal_collection_h

#include "object.h"
/* object types for storing collections of data & their APIs. */

#define BRANCH_FACTOR 0x040ul
#define INDEX_MASK    0x03ful
#define LEVEL_SHIFT   0x006ul
#define MAX_SHIFT     0x030ul
#define MAX_LEVEL     0x008ul

struct Bits {
  Obj     obj;
  byte_t* data;
  size_t  arity;
};

struct String {
  Obj    obj;
  char*  data;
  size_t arity;
};

struct Tuple {
  Obj    obj;
  Value* data;
  size_t arity;
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
};

struct Map {
  Obj      obj;
  MapNode* root;  // key/value pairs stored in underlying structure
  size_t   arity; // total number of key/value pairs
};

// node types
struct VecNode {
  Obj      obj;
  Obj**    children;
  size_t   count;
};

struct VecLeaf {
  Obj   obj;
  Value slots[BRANCH_FACTOR];
};

struct MapNode {
  Obj      obj;
  Obj**    children;
  uint64_t bitmap;
};

struct MapLeaf {
  Obj      obj;
  MapLeaf* next; // used for collision resolution.
  Value    key;
  Value    val;
};

// globals
// empty sigletons
extern Bits   EmptyBits;
extern String EmptyString;
extern Tuple  EmptyTuple;
extern List   EmptyList;
extern Vector EmptyVector;
extern Map    EmptyMap;

// type objects
extern struct Type BitsType, StringType, TupleType, ListType,
  VectorType, VecNodeType, VecLeafType,
  MapType, MapNodeType, MapLeafType;

// external AP
// constructors
Bits*     newBits(void* data, size_t count, int flags);
String*   newString(char* chars, size_t count, int flags);
Tuple*    newTuple(size_t n, Value* vs);
List*     newList(Value head, List* tail);
Vector*   newVector(size_t n, Value* vs);
VecNode*  newVecNode(Obj** children, size_t n, int flags);
VecLeaf*  newVecLeaf(Value* tail);
Map*      newMap(size_t n, Value* kvs);


// collection interfaces
size_t    getElSize(Bits* b);
bool      fitsElSize(Bits* b, int i);
Bits*     cloneBits(Bits* b);
Value     bitsGet(Bits* b, size_t n);
Bits*     bitsAdd(Bits* b, int i);
Bits*     bitsSet(Bits* b, size_t n, int i);
Bits*     bitsDel(Bits* b, size_t n);

Encoding  getEncoding(String* s);
String*   cloneString(String* s);
Value     strGet(String* s, size_t n);
String*   strAdd(String* s, Glyph g);
String*   strSet(String* s, size_t n, Glyph g);
String*   strDel(String* s, size_t n);

Tuple*    cloneTuple(Tuple* t);
Value     tupleGet(Tuple* t, size_t n);
Tuple*    tupleAdd(Tuple* t, Value x);
Tuple*    tupleSet(Tuple* t, size_t n, Value x);
Tuple*    tupleDel(Tuple* t, size_t n);

Value     listGet(List* l, size_t n);
List*     listAdd(List* l, Value x);
List*     listSet(List* l, size_t n, Value x);
List*     listDel(List* l, size_t n);

size_t    getShift(void* p);
size_t    setShift(void* p, size_t sh);
void*     unfreeze(void* p);
void      freeze(void* p);

Value     mapGet(Map* m, Value k);
Map*      mapAdd(Map* m, Value k, Value v);
Map*      mapSet(Map* m, Value k, Value v);
Map*      mapDel(Map* m, Value k);

Value     vecGet(Vector* v, size_t i);
Vector*   vecAdd(Vector* v, Value x);
Vector*   vecSet(Vector* v, size_t i, Value x);
Vector*   vecDel(Vector* v);

#endif
