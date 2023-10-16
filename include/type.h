#ifndef rascal_type_h
#define rascal_type_h

#include "object.h"

/* globals & utilites for working with types. */
// C types
// generics
#include "tpl/declare.h"

TABLE_TYPE(TypeSet, Type*, Type*);

// categorization of types. Used to determine membership and compare specificity of types
typedef enum {
  BOTTOM_TYPE_KIND, DATA_TYPE_KIND, DATA_UNION_KIND, ABSTRACT_TYPE_KIND,
  ABSTRACT_UNION_KIND, TOP_TYPE_KIND,
} Kind;

struct Vtable {
  size_t    valSize;         // base value size  (0 - 8 bytes)
  size_t    objSize;         // base object size (32+ bytes)
  uintptr_t tag;             // tag used for values of given type
  SizeFn    sizeOf;          // 
  TraceFn   trace;
  FreeFn    free;
  HashFn    hash;
  EgalFn    equal;
  OrdFn     order;
  OrdFn     rank;
};

struct Type {
  Obj       obj;
  Symbol*   name;
  Type*     parent;        // abstract parent type
  Function* ctor;          // constructor for values of this type
  Vtable*   vTable;        // runtime and internal methods for types with concrete values
  uintptr_t idno;          // unique identifier for this type (similar to symbol idno)
  TypeSet   members;       // set of member types (if this is a union type)
};

// globals
// id numbers for builtin types
enum {
  // fucked up types
  TOP=1, BOTTOM, UNIT, TERM, TYPE, 

  // numeric types
  FLOAT, ARITY, SMALL, BIG, NUMBER, REAL, RATIONAL, INTEGER,

  // miscellaneous
  BOOLEAN, GLYPH, STREAM,

  // namespaces
  SYMBOL, SCOPE, ENVIRONMENT, BINDING, UPVALUE, 

  // collections
  BITS, STRING, TUPLE, LIST, VECTOR, VEC_NODE, VEC_LEAF, MAP, MAP_NODE, MAP_LEAF,

  // functions
  FUNCTION, METHOD_TABLE, METHOD_MAP, METHOD_NODE, METHOD, NATIVE, CLOSURE,

  // runtime, execution, & control
  CHUNK, CONTROL,
};

// fucked up types and TypeType
extern Type NoneType, AnyType, TermType, TypeType;

// external API
// accessors
Kind getKind(Type* type);

// constructors
Type*   newStructType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newRecordType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newAbstractType(Type* parent, Symbol* name);
Type*   newUnionType(Type* parent, Symbol* name, size_t count, Type** members);

// utilities
Tuple*  rankTypes(Tuple* sig);
int     orderSigs(Tuple* sigx, Tuple* sigy);

// initialization
void   initializeBuiltinTypes(void);

#endif
