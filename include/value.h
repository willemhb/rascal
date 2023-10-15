#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// builtin type codes
enum {
  // dummy (for when a typecode needs to be passed but 'none' is valid)
  NOTYPE,

  // user types
  // value types
  FLOAT, ARITY, SMALL, BOOLEAN, UNIT, GLYPH, OBJECT,

  // object types (object is not a true type so it's skipped)
  SYMBOL=OBJECT, FUNCTION, TYPE, BINDING, STREAM, BIG, BITS, LIST, VECTOR, MAP,

  // internal types
  METHOD_TABLE, NATIVE, CHUNK, CLOSURE, CONTROL, SCOPE, NAMESPACE, ENVIRONMENT, UPVALUE,

  // node types
  VEC_NODE, VEC_LEAF, MAP_NODE, MAP_LEAF,

  // abstract types
  BOTTOM, TOP, TERM, NUMBER, REAL, RATIONAL, INTEGER,
};

// builtin datatype objects
extern Type FloatType, ArityType, SmallType, BooleanType, UnitType, GlyphType,
  SymbolType, FunctionType, TypeType, BindingType, StreamType, BigType, BitsType,
  ListType, VectorType, MapType, MethodTableType, NativeType, ChunkType, ClosureType,
  ControlType, ScopeType, NameSpaceType, EnvironmentType, UpValueType, VecNodeType,
  VecLeafType, MapNodeType, MapLeafType;

// builtin abstract and union types
extern Type NoneType, AnyType, TermType, NumberType, RealType, RationalType, IntegerType;

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

#define tag(x) _Generic((x),                    \
                        Float:tagFloat,         \
                        Arity:tagArity,         \
                        Small:tagSmall,         \
                        Boolean:tagBoolean,     \
                        Glyph:tagGlyph,         \
                        default:tagObj)(x)

#define AS_FLOAT(x)        ((Float)wordToDouble(x))
#define AS_ARITY(x)        ((x) & VAL_MASK)
#define AS_SMALL(x)        ((Small)((x) & SMALL_MASK))
#define AS_BOOL(x)         ((x) == TRUE)
#define AS_GLYPH(x)        ((Glyph)((x) & SMALL_MASK))
#define AS_PTR(x)          ((void*)((x) & VAL_MASK))

#define AS_OBJ(x)          ((Obj*)AS_PTR(x))
#define AS_SYMBOL(x)       ((Symbol*)AS_PTR(x))
#define AS_FUNCTION(x)     ((Function*)AS_PTR(x))
#define AS_TYPE(x)         ((Type*)AS_PTR(x))
#define AS_BINDING(x)      ((Binding*)AS_PTR(x))
#define AS_STREAM(x)       ((Stream*)AS_PTR(x))
#define AS_BIG(x)          ((Big*)AS_PTR(x))
#define AS_BITS(x)         ((Bits*)AS_PTR(x))
#define AS_LIST(x)         ((List*)AS_PTR(x))
#define AS_TUPLE(x)
#define AS_VECTOR(x)       ((Vector*)AS_PTR(x))
#define AS_MAP(x)          ((Map*)AS_PTR(x))
#define AS_METHOD_TABLE(x) ((MethodTable*)AS_PTR(x))
#define AS_NATIVE(x)       ((Native*)AS_PTR(x))
#define AS_CHUNK(x)        ((Chunk*)AS_PTR(x))
#define AS_CLOSURE(x)      ((Closure*)AS_PTR(x))
#define AS_CONTROL(x)      ((Control*)AS_PTR(x))
#define AS_SCOPE(x)        ((Scope*)AS_PTR(x))
#define AS_NAMESPACE(x)    ((NameSpace*)AS_PTR(x))
#define AS_ENVIRON(x)      ((Environment*)AS_PTR(x))
#define AS_UPVALUE(x)      ((UpValue*)AS_PTR(x))
#define AS_VEC_NODE(x)     ((VecNode*)AS_PTR(x))
#define AS_VEC_LEAF(x)     ((VecLeaf*)AS_PTR(x))
#define AS_MAP_NODE(x)     ((MapNode*)AS_PTR(x))
#define AS_MAP_LEAF(x)     ((MapLeaf*)AS_PTR(x))

#define IS_FLOAT(x)        hasType(x, &FloatType)
#define IS_ARITY(x)        hasType(x, &ArityType)
#define IS_SMALL(x)        hasType(x, &SmallType)
#define IS_BOOLEAN(x)      hasType(x, &BooleanType)
#define IS_UNIT(x)         hasType(x, &UnitType)
#define IS_GLYPH(x)        hasType(x, &GlyphType)
#define IS_OBJ(x)          (((x) & TAG_MASK) == OBJ_TAG)
#define IS_SYMBOL(x)       hasType(x, &SymbolType)
#define IS_FUNCTION(x)     hasType(x, &FunctionType)
#define IS_TYPE(x)         hasType(x, &TypeType)
#define IS_BINDING(x)      hasType(x, &BindingType)
#define IS_STREAM(x)       hasType(x, &StreamType)
#define IS_BIG(x)          hasType(x, &BigType)
#define IS_BITS(x)         hasType(x, &BitsType)
#define IS_LIST(x)         hasType(x, &ListType)
#define IS_VECTOR(x)       hasType(x, &VectorType)
#define IS_MAP(x)          hasType(x, &MapType)
#define IS_METHOD_TABLE(x) hasType(x, &MethodTableType)
#define IS_NATIVE(x)       hasType(x, &NativeType)
#define IS_CHUNK(x)        hasType(x, &ChunkType)
#define IS_CLOSURE(x)      hasType(x, &ClosureType)
#define IS_CONTROL(x)      hasType(x, &ControlType)
#define IS_SCOPE(x)        hasType(x, &ScopeType)
#define IS_NAMESPACE(x)    hasType(x, &NameSpaceType)
#define IS_ENVIRONMENT(x)  hasType(x, &EnvironmentType)
#define IS_UPVALUE(x)      hasType(x, &UpValueType)
#define IS_VECNODE(x)      hasType(x, &VecNodeType)
#define IS_VECLEAF(x)      hasType(x, &VecLeafType)
#define IS_MAPNODE(x)      hasType(x, &MapNodeType)
#define IS_MAPLEAF(x)      hasType(x, &MapLeafType)

#define typeOf(v)         generic2(typeOf, v, v)
#define sizeOf(v)         generic2(sizeOf, v, v)
#define hasType(v, t)     generic2(hasType, v, v, t)

Value tagFloat(Float x);
Value tagArity(Arity x);
Value tagSmall(Small x);
Value tagBoolean(Boolean x);
Value tagGlyph(Glyph x);
Value tagObj(void* x);

Type*  typeOfVal(Value x);
Type*  typeOfObj(void* p);
size_t sizeOfVal(Value x);
size_t sizeOfObj(void* p);
bool   hasTypeVal(Value x, Type* type);
bool   hasTypeObj(void* p, Type* type);

void   initializeBuiltinTypes(void);

#endif
