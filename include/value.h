#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// core rascal types
// basic value types
typedef uintptr_t       Value;    // standard tagged value representation (NaN boxed)
typedef struct Obj      Obj;      // generic object

// immediate types
typedef double          Float;    // ieee754-64 floating point number
typedef uintptr_t       Arity;    // 48-bit unsigned integer
typedef int             Small;    // 32-bit integer
typedef bool            Boolean;  // boolean
typedef int             Glyph;    // unicode codepoint

// object types
// user types
typedef struct Symbol   Symbol;   // interned symbol
typedef struct Function Function; // generic function object
typedef struct Type     Type;     // first-class representation of a rascal type
typedef struct Binding  Binding;  // object for storing variable bindings
typedef struct Stream   Stream;   // IO stream
typedef struct Big      Big;      // arbitrary precision integer
typedef struct Bits     Bits;     // compact binary data
typedef struct List     List;     // immutable linked list
typedef struct Vector   Vector;   // clojure-like vector
typedef struct Map      Map;      // clojure-like hashmap

// internal types
// vm types
typedef struct MethodTable MethodTable; // core of multimethod implementation
typedef struct Native      Native;      // native function or special form
typedef struct Chunk       Chunk;       // compiled code
typedef struct Closure     Closure;     // packages a chunk/namespace with names
typedef struct Scope       Scope;       // naming context
typedef struct Environment Environment; // naming context plus values

// node types
typedef struct VecNode     VecNode;
typedef struct VecLeaf     VecLeaf;
typedef struct MapNode     MapNode;
typedef struct MapLeaf     MapLeaf;

// various & sundry enums
typedef enum {
  // dummy (for when a typecode needs to be passed but 'none' is valid)
  NOTYPE,

  // user types
  // value types
  FLOAT,
  ARITY,
  SMALL,
  BOOLEAN,
  UNIT,
  GLYPYH,
  OBJECT,

  // object types (object is not a true type so it's skipped)
  SYMBOL=OBJECT,
  FUNCTION,
  TYPE,
  BINDING,
  STREAM,
  BIG,
  BITS,
  LIST,
  VECTOR,
  MAP,

  // internal types
  METHOD_TABLE,
  NATIVE,
  CHUNK,
  CLOSURE,
  SCOPE,
  ENVIRONMENT,

  // node types
  VEC_NODE,
  VEC_LEAF,
  MAP_NODE,
  MAP_LEAF,
} TypeCode;

// builtin datatype objects
extern Type FloatType, ArityType, SmallType, BooleanType, UnitType, GlyphType,
  SymbolType, FunctionType, TypeType, BindingType, StreamType, BigType, BitsType,
  ListType, VectorType, MapType, MethodTableType, NativeType, ChunkType, ClosureType,
  ScopeType, EnvironmentType, VecNodeType, VecLeafType, MapNodeType, MapLeafType;

#define NUM_TYPES (MAP_LEAF+1)

// tags and such
#define ARITY_TAG   0x7ffc000000000000UL
#define SMALL_TAG   0x7ffd000000000000UL
#define BOOL_TAG    0x7ffe000000000000UL
#define NUL_TAG     0x7fff000000000000UL
#define GLYPH_TAG   0xfffc000000000000UL
#define OBJ_TAG     0xfffd000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL
#define SMALL_MASK  0x00000000ffffffffUL

#define TRUE        (BOOL_TAG | 1UL)
#define FALSE       (BOOL_TAG | 0UL)
#define NUL         (NUL_TAG  | 0UL)
#define NOTHING     (NUL_TAG  | 1UL) // invalid value marker

#define TAG_FLOAT(number)     doubleToWord(number)
#define TAG_ARITY(number)     (((Value)(number)) & VAL_MASK | ARITY_TAG)
#define TAG_SMALL(number)     (((Value)(number))  | SMALL_TAG)
#define TAG_BOOL(boolean)     ((boolean) ? TRUE : FALSE)
#define TAG_GLYPH(glyph)      (((Value)(glyph))   | GLYPH_TAG)
#define TAG_OBJ(pointer)      (((Value)(pointer)) | OBJ_TAG)

#define AS_FLOAT(value)        ((Float)wordToDouble(value))
#define AS_ARITY(value)        ((value) & VAL_MASK)
#define AS_SMALL(value)        ((Small)((value) & SMALL_MASK))
#define AS_BOOL(value)         ((value) == TRUE)
#define AS_GLYPH(value)        ((Glyph)((value) & SMALL_MASK))
#define AS_PTR(value)          ((void*)((value) & VAL_MASK))

#define AS_OBJ(value)          ((Obj*)AS_PTR(value))
#define AS_SYMBOL(value)       ((Symbol*)AS_PTR(value))
#define AS_FUNCTION(value)     ((Function*)AS_PTR(value))
#define AS_TYPE(value)         ((Type*)AS_PTR(value))
#define AS_BINDING(value)      ((Binding*)AS_PTR(value))
#define AS_STREAM(value)       ((Stream*)AS_PTR(value))
#define AS_BIG(value)          ((Big*)AS_PTR(value))
#define AS_BITS(value)         ((Bits*)AS_PTR(value))
#define AS_LIST(value)         ((List*)AS_PTR(value))
#define AS_VECTOR(value)       ((Vector*)AS_PTR(value))
#define AS_MAP(value)          ((Map*)AS_PTR(value))
#define AS_METHOD_TABLE(value) ((MethodTable*)AS_PTR(value))
#define AS_NATIVE(value)       ((Native*)AS_PTR(value))
#define AS_CHUNK(value)        ((Chunk*)AS_PTR(value))
#define AS_CLOSURE(value)      ((Closure*)AS_PTR(value))
#define AS_SCOPE(value)        ((Scope*)AS_PTR(value))
#define AS_ENVIRONMENT(value)  ((Environment*)AS_PTR(value))
#define AS_VEC_NODE(value)     ((VecNode*)AS_PTR(value))
#define AS_VEC_LEAF(value)     ((VecLeaf*)AS_PTR(value))
#define AS_MAP_NODE(value)     ((MapNode*)AS_PTR(value))
#define AS_MAP_LEAF(value)     ((MapLeaf*)AS_PTR(value))

#define IS_FLOAT(value)        hasType(value, &FloatType)
#define IS_ARITY(value)        hasType(value, &ArityType)
#define IS_SMALL(value)        hasType(value, &SmallType)
#define IS_BOOLEAN(value)      hasType(value, &BooleanType)
#define IS_UNIT(value)         hasType(value, &UnitType)
#define IS_GLYPH(value)        hasType(value, &GlyphType)
#define IS_OBJ(value)          hasValueType(value, OBJECT)
#define IS_SYMBOL(value)       hasType(value, SYMBOL)
#define IS_FUNCTION(value)     hasType(value, FUNCTION)
#define IS_TYPE(value)         hasType(value, TYPE)
#define IS_BINDING(value)      hasType(value, BINDING)
#define IS_STREAM(value)       hasType(value, STREAM)
#define IS_BITS(value)         hasType(value, BITS)
#define IS_LIST(value)         hasType(value, LIST)
#define IS_METHOD_TABLE(value) hasType(value, METHOD_TABLE)
#define IS_NATIVE(value)       hasType(value, NATIVE)
#define IS_CHUNK(value)        hasType(value, CHUNK)
#define IS_CLOSURE(value)      hasType(value, CLOSURE)
#define IS_SCOPE(value)        hasType(value, SCOPE)
#define IS_ENVIRONMENT(value)  hasType(value, ENVIRONMENT)

#include "tpl/declare.h"

ARRAY_TYPE(Values, Value);
TABLE_TYPE(Annotations, annotations, Value, Obj*);

#define typeOf(v) _Generic((v),                     \
                           Value:typeOfVal,         \
                           default:typeOfObj)(v)

#define typeCode(v) typeOf(v)->code

bool     hasValType(Value value, TypeCode type);
bool     hasVmType(Value value, TypeCode type);

void     printValues(FILE* ios, Values* values);
Type*    typeOfVal(Value value);
Type*    typeOfObj(void* ptr);
bool     equalValues(Value x, Value y);
uint64_t hashValue(Value x);
void     printValue(FILE* ios, Value x);
void     printLine(FILE* ios, Value x);

#endif
