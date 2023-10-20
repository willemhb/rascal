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
  Value* slots;
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
  MapLeaf* next; // used for collision resolution (convenient but inefficient)
  Value    key;
  Value    val;
};

// globals
// type objects
extern struct Type BitsType, StringType, TupleType,
  ListType, VectorType, VecNodeType, VecLeafType,
  MapType, MapNodeType, MapLeafType;

// external AP
// constructors
Bits*     new_bits(void* data, size_t count, flags_t fl);
String*   new_str(char* chars, size_t count, flags_t fl);
Tuple*    new_tuple(size_t n, Value* vs);
List*     new_list(size_t n, Value* vs);
Vector*   new_vec(size_t n, Value* vs);
VecNode*  new_vecn(Obj** children, size_t n, int flags);
VecLeaf*  new_vecl(Value* tail);
Map*      new_map(size_t n, Value* kvs);

// convenience constructors
List*     cons(Value head, List* tail);

// collection interfaces
size_t    get_elsize(Bits* b);
bool      fits_elsize(Bits* b, int i);
Bits*     clone_bits(Bits* b);
Value     bits_get(Bits* b, size_t n);
Bits*     bits_add(Bits* b, int i);
Bits*     bits_set(Bits* b, size_t n, int i);
Bits*     bits_del(Bits* b, size_t n);

Encoding  get_encoding(String* s);
String*   clone_str(String* s);
Value     str_get(String* s, size_t n);
String*   str_add(String* s, Glyph g);
String*   str_set(String* s, size_t n, Glyph g);
String*   str_del(String* s, size_t n);

Tuple*    clone_tuple(Tuple* t);
Value     tuple_get(Tuple* t, size_t n);
Tuple*    tuple_add(Tuple* t, Value x);
Tuple*    tuple_set(Tuple* t, size_t n, Value x);
Tuple*    tuple_del(Tuple* t, size_t n);

Value     list_get(List* l, size_t n);
List*     list_add(List* l, Value x);
List*     list_set(List* l, size_t n, Value x);
List*     list_del(List* l, size_t n);

size_t    get_shift(void* p);
size_t    set_shift(void* p, size_t sh);
void*     unfreeze(void* p);
void      freeze(void* p);

Value     map_get(Map* m, Value k);
bool      map_has(Map* m, Value k);
Map*      map_add(Map* m, Value k, Value v);
Map*      map_set(Map* m, Value k, Value v);
Map*      map_del(Map* m, Value k);
Map*      merge_maps(Map* dst, Map* src);

Value     vec_get(Vector* v, size_t i);
Vector*   vec_add(Vector* v, Value x);
Vector*   vec_set(Vector* v, size_t i, Value x);
Vector*   vec_del(Vector* v);

// initialization
void init_global_singletons(void);

#endif
