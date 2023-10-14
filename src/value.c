#include "util/hashing.h"

#include "object.h"
#include "value.h"

// external API
Value tagFloat(Float x) {
  return doubleToWord(x);
}

Value tagArity(Arity x) {
  return (x & VAL_MASK) | ARITY_TAG;
}

Value tagSmall(Small x) {
  return ((Value)x) | SMALL_TAG;
}

Value tagBoolean(Boolean x) {
  return x ? TRUE : FALSE;
}

Value tagGlyph(Glyph x) {
  return ((Value)x) | GLYPH_TAG;
}

Value tagObj(void* x) {
  return ((Value)x) | OBJ_TAG;
}

Type* typeOfVal(Value value) {
  Type* out;
  
  switch (value & TAG_MASK) {
    case ARITY_TAG: out = &ArityType;               break;
    case SMALL_TAG: out = &SmallType;               break;
    case NUL_TAG:   out = &UnitType;                break;
    case BOOL_TAG:  out = &BooleanType;             break;
    case GLYPH_TAG: out = &GlyphType;               break;
    case OBJ_TAG:   out = typeOfObj(AS_PTR(value)); break;
    default:        out = &FloatType;               break;
  }

  return out;
}

Type* typeOfObj(void* ptr) {
  assert(ptr != NULL);
  Obj*  obj = ptr;
  Type* out = obj->type;

  return out;
}

static bool isInstance(Type* vt, Type* type) {
  bool out;
  
  switch (type->kind) {
    case BOTTOM_KIND:     out = false;      break;
    case TOP_KIND:        out = true;       break;
    case DATA_TYPE_KIND:  out = vt == type; break;
    case UNION_TYPE_KIND:
      if (type->left)
        out = isInstance(vt, type->left);

      if (!out && type->right)
        out = isInstance(vt, type->right);

      break;
    case ABSTRACT_TYPE_KIND:
      vt = vt->parent;

      for (;vt != &AnyType && !out; vt=vt->parent)
        out = vt == type;
      break;
  }

  return out;
}

bool hasTypeVal(Value x, Type* type) {
  return isInstance(typeOf(x), type);
}

bool hasTypeObj(void* p, Type* type) {
  return isInstance(typeOf(p), type);
}

// isa implementations
// builtin type objects
// immediate types
// float type
extern void     printFloat(FILE* ios, Value value);
extern uint64_t hashFloat(Value value);
extern int      orderFloats(Value x, Value y);

Vtable FloatTable = {
  .valSize =sizeof(Float),
  .hash    =hashFloat,
  .print   =printFloat,
  .order   =orderFloats
};

Type FloatType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&RealType,
  .vTable=&FloatTable,
  .idno  =FLOAT,
  .kind  =DATA_TYPE_KIND,
};

// arity type
extern void     printArity(FILE* ios, Value value);
extern uint64_t hashArity(Value x);
extern int      orderArities(Value x, Value y);

Vtable ArityTable = {
  .valSize=sizeof(Arity),
  .tag    =ARITY_TAG,
  .hash   =hashArity,
  .print  =printArity,
  .order  =orderArities
};

Type ArityType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&NumberType,
  .vTable=&ArityTable,
  .idno  =ARITY,
  .kind  =DATA_TYPE_KIND,
};

// small type
extern void     printSmall(FILE* ios, Value value);
extern uint64_t hashSmall(Value x);
extern int      orderArities(Value x, Value y);

Vtable SmallTable = {
  .valSize=sizeof(Small),
  .tag    =SMALL_TAG,
  .hash   =hashSmall,
  .print  =printSmall,
  .order  =orderArities
};

Type SmallType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&IntegerType,
  .vTable=&SmallTable,
  .idno  =SMALL,
  .kind  =DATA_TYPE_KIND,
};

// boolean type
extern void     printBoolean(FILE* ios, Value value);
extern uint64_t hashBoolean(Value value);
extern int      orderBooleans(Value x, Value y);

Vtable BooleanTable = {
  .valSize=sizeof(Boolean),
  .tag    =BOOL_TAG,
  .hash   =hashBoolean,
  .print  =printBoolean,
  .order  =orderBooleans
};

Type BooleanType = {

  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&TermType,
  .vTable=&BooleanTable,
  .idno  =BOOLEAN,
  .kind  =DATA_TYPE_KIND,
};

// unit type
extern void     printUnit(FILE* ios, Value value);
extern uint64_t hashUnit(Value x);
extern int      orderUnits(Value x, Value y);

Vtable UnitTable = {
  .valSize=sizeof(Value),
  .tag    =NUL_TAG,
  .hash   =hashUnit,
  .print  =printUnit,
  .order  =orderUnits
};

Type UnitType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&UnitTable,
  .idno  =UNIT
};

// glyph type
extern void     printGlyph(FILE* ios, Value value);
extern uint64_t hashGlyph(Value value);
extern int      orderGlyphs(Value x, Value y);

Vtable GlyphTable = {
  .valSize=sizeof(Glyph),
  .tag    =GLYPH_TAG,
  .hash   =hashGlyph,
  .print  =printGlyph
};

Type GlyphType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&GlyphTable,
  .idno  =GLYPH
};

// symbol type
extern void     traceSymbol(void* p);
extern void     freeSymbol(void* p);
extern void     printSymbol(FILE* ios, Value value);
extern uint64_t hashSymbol(Value value);
extern int      orderSymbols(Value x, Value y);

Vtable SymbolTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Symbol),
  .tag    =OBJ_TAG,
  .trace  =traceSymbol,
  .free   =freeSymbol,
  .print  =printSymbol,
  .hash   =hashSymbol,
  .order  =orderSymbols
};

Type SymbolType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&SymbolTable,
  .idno  =SYMBOL
};

// function type
extern void printFunction(FILE* ios, Value value);
extern void traceFunction(void* p);    

Vtable FunctionTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Function),
  .tag    =OBJ_TAG,
  .trace  =traceFunction,
  .print  =printFunction
};

Type FunctionType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&FunctionTable,
  .idno  =FUNCTION
};

// type type
extern void     traceType(void* p);
extern void     freeType(void* p);
extern void     printType(FILE* ios, Value value);
extern uint64_t hashType(Value v);
extern int      orderTypes(Value x, Value y);

Vtable TypeTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Type),
  .tag    =OBJ_TAG,
  .trace  =traceType,
  .free   =freeType,
  .print  =printType,
  .hash   =hashType,
  .order  =orderTypes
};

Type TypeType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&TypeTable,
  .idno  =TYPE
};

// binding type
extern void traceBinding(void* p);

Vtable BindingTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Binding),
  .tag    =OBJ_TAG,
  .trace  =traceBinding
};

Type BindingType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&BindingTable,
  .idno  =BINDING
};

// stream type
extern void freeStream(void* p);

Vtable StreamTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Stream),
  .tag    =OBJ_TAG,
  .free   =freeStream
};

Type StreamType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&StreamTable,
  .idno  =STREAM
};

// Big type
extern void     printBig(FILE* ios, Value value);
extern uint64_t hashBig(Value x);
extern bool     equalBigs(Value x, Value y);
extern int      orderBigs(Value x, Value y);

Vtable BigTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Big),
  .tag    =OBJ_TAG,
  .free   =freeStream,
  .print  =printBig,
  .hash   =hashBig,
  .equal  =equalBigs,
  .order  =orderBigs
};

Type BigType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&IntegerType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&BigTable,
  .idno  =BIG
};

// Bits type
extern void     freeBits(void* p);
extern void     printBits(FILE* ios, Value value);
extern uint64_t hashBits(Value x);
extern bool     equalBits(Value x, Value y);
extern int      orderBits(Value x, Value y);

Vtable BitsTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Bits),
  .tag    =OBJ_TAG,
  .free   =freeBits,
  .print  =printBits,
  .hash   =hashBits,
  .equal  =equalBits,
  .order  =orderBits
};

Type BitsType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&BitsTable,
  .idno  =BITS
};

// List type
extern void     traceList(void* p);
extern void     printList(FILE* ios, Value value);
extern uint64_t hashList(Value x);
extern bool     equalLists(Value x, Value y);
extern int      orderLists(Value x, Value y);

Vtable ListTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(List),
  .tag    =OBJ_TAG,
  .trace  =traceList,
  .print  =printList,
  .hash   =hashList,
  .equal  =equalLists,
  .order  =orderLists
};

Type ListType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&ListTable,
  .idno  =LIST
};

// Vector type
extern size_t   sizeOfVector(void* p);
extern void     traceVector(void* p);
extern void     printVector(FILE* ios, Value value);
extern uint64_t hashVector(Value x);
extern bool     equalVectors(Value x, Value y);
extern int      orderVectors(Value x, Value y);

Vtable VectorTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Vector),
  .tag    =OBJ_TAG,
  .sizeOf =sizeOfVector,
  .trace  =traceVector,
  .print  =printVector,
  .hash   =hashVector,
  .equal  =equalVectors,
  .order  =orderVectors
};

Type VectorType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&VectorTable,
  .idno  =VECTOR
};

// Map type
extern void     traceMap(void* p);
extern void     printMap(FILE* ios, Value value);
extern uint64_t hashMap(Value x);
extern bool     equalMaps(Value x, Value y);
extern int      orderMaps(Value x, Value y);

Vtable MapTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Map),
  .tag    =OBJ_TAG,
  .trace  =traceMap,
  .print  =printMap,
  .hash   =hashMap,
  .equal  =equalMaps,
  .order  =orderMaps
};

Type MapType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&MapTable,
  .idno  =MAP
};

// MethodTable type
extern void traceMethodTable(void* p);
extern void freeMethodTable(void* p);

Vtable MethodTableTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(MethodTable),
  .tag    =OBJ_TAG,
  .trace  =traceMethodTable,
  .free   =freeMethodTable
};

Type MethodTableType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&MethodTableTable,
  .idno  =METHOD_TABLE
};

// Native type
Vtable NativeTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Native),
  .tag    =OBJ_TAG
};

Type NativeType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&NativeTable,
  .idno  =NATIVE
};

// Chunk type
extern void traceChunk(void* p);
extern void freeChunk(void* p);

Vtable ChunkTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Chunk),
  .tag    =OBJ_TAG,
  .trace  =traceChunk,
  .free   =freeChunk
};

Type ChunkType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&ChunkTable,
  .idno  =CHUNK
};

// Closure type
extern void traceClosure(void* p);
extern void freeClosure(void* p);

Vtable ClosureTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Closure),
  .tag    =OBJ_TAG,
  .trace  =traceClosure,
  .free   =freeClosure
};

Type ClosureType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&ClosureTable,
  .idno  =CLOSURE
};

// Control type
extern void traceControl(void* p);
extern void freeControl(void* p);

Vtable ControlTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Control),
  .tag    =OBJ_TAG,
  .trace  =traceControl,
  .free   =freeControl
};

Type ControlType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&ControlTable,
  .idno  =CONTROL
};

// Scope type
extern void traceScope(void* p);
extern void freeScope(void* p);

Vtable ScopeTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Scope),
  .tag    =OBJ_TAG,
  .trace  =traceScope,
  .free   =freeScope
};

Type ScopeType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&ScopeTable,
  .idno  =SCOPE
};

// NameSpace type
extern void traceNameSpace(void* p);

Vtable NameSpaceTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(NameSpace),
  .tag    =OBJ_TAG,
  .trace  =traceNameSpace
};

Type NameSpaceType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&NameSpaceTable,
  .idno  =NAMESPACE
};

// Environment type
extern void traceEnvironment(void* p);
extern void freeEnvironment(void* p);

Vtable EnvironmentTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(Environment),
  .tag    =OBJ_TAG,
  .trace  =traceEnvironment,
  .free   =freeEnvironment,
};

Type EnvironmentType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&EnvironmentTable,
  .idno  =ENVIRONMENT
};

// UpValue type
extern void traceUpValue(void* p);

Vtable UpValueTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(UpValue),
  .tag    =OBJ_TAG,
  .trace  =traceUpValue
};

Type UpValueType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&UpValueTable,
  .idno  =UPVALUE
};

// VecNode type
extern void traceVecNode(void* p);
extern void freeVecNode(void* p);

Vtable VecNodeTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(VecNode),
  .tag    =OBJ_TAG,
  .trace  =traceVecNode,
  .free   =freeVecNode
};

Type VecNodeType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&VecNodeTable,
  .idno  =VEC_NODE
};

// VecLeaf type
extern void traceVecLeaf(void* p);
extern void freeVecLeaf(void* p);

Vtable VecLeafTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(VecNode),
  .tag    =OBJ_TAG,
  .trace  =traceVecLeaf,
  .free   =freeVecLeaf
};

Type VecLeafType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&VecLeafTable,
  .idno  =VEC_LEAF
};

// MapNode type
extern void traceMapNode(void* p);
extern void freeMapNode(void* p);

Vtable MapNodeTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(MapNode),
  .tag    =OBJ_TAG,
  .trace  =traceMapNode,
  .free   =freeMapNode
};

Type MapNodeType = {
  .obj={
    .next   =NULL,
    .annot  =&emptyMap,
    .type   =&TypeType,
    .hash   =0,
    .hashed =false,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
    .black  =false
  },

  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&MapNodeTable,
  .idno  =MAP_NODE,
};

// MapLeaf type
extern void traceMapLeaf(void* p);
extern void freeMapLeaf(void* p);

Vtable MapLeafTable = {
  .valSize=sizeof(Obj*),
  .objSize=sizeof(MapLeaf),
  .tag    =OBJ_TAG,
  .trace  =traceMapLeaf,
  .free   =freeMapLeaf
};

Type MapLeafType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .kind  =DATA_TYPE_KIND,
  .vTable=&MapLeafTable,
  .idno  =MAP_LEAF,
};

// abstract types
Type NoneType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&AnyType,
  .idno     =BOTTOM,
  .kind     =BOTTOM_KIND,
};

Type AnyType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&AnyType,
  .idno     =TOP,
  .kind     =TOP_KIND,
};

Type TermType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&AnyType,
  .idno     =TERM,
  .kind     =ABSTRACT_TYPE_KIND,
};

Type NumberType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&TermType,
  .idno     =NUMBER,
  .kind     =ABSTRACT_TYPE_KIND,
};

Type RealType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&NumberType,
  .idno     =REAL,
  .kind     =ABSTRACT_TYPE_KIND,
};

Type RationalType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&RealType,
  .idno     =RATIONAL,
  .kind     =ABSTRACT_TYPE_KIND,
};

Type IntegerType = {
  .obj={
    .annot  =&emptyMap,
    .type   =&TypeType,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent   =&RealType,
  .kind     =ABSTRACT_TYPE_KIND,
  .idno     =RATIONAL,
};

#include "vm.h"
#include "environment.h"

void initializeBuiltinTypes(void) {
  struct { char* name; Type* type; } types[] = {
    [FLOAT]        = { "Float",       &FloatType       },
    [ARITY]        = { "Arity",       &ArityType       },
    [SMALL]        = { "Small",       &SmallType       },
    [BOOLEAN]      = { "Boolean",     &BooleanType     },
    [UNIT]         = { "Unit",        &UnitType        },
    [GLYPH]        = { "Glyph",       &GlyphType       },
    [SYMBOL]       = { "Symbol",      &SymbolType      },
    [FUNCTION]     = { "Function",    &FunctionType    },
    [TYPE]         = { "Type",        &TypeType        },
    [BINDING]      = { "Binding",     &BindingType     },
    [STREAM]       = { "Stream",      &StreamType      },
    [BIG]          = { "Big",         &BigType         },
    [BITS]         = { "Bits",        &BitsType        },
    [LIST]         = { "List",        &ListType        },
    [VECTOR]       = { "Vector",      &VectorType      },
    [MAP]          = { "Map",         &MapType         },
    [METHOD_TABLE] = { "MethodTable", &MethodTableType },
    [NATIVE]       = { "Native",      &NativeType      },
    [CHUNK]        = { "Chunk",       &ChunkType       },
    [CLOSURE]      = { "Closure",     &ClosureType     },
    [CONTROL]      = { "Control",     &ControlType     },
    [SCOPE]        = { "Scope",       &ScopeType       },
    [NAMESPACE]    = { "NameSpace",   &NameSpaceType   },
    [ENVIRONMENT]  = { "Environment", &EnvironmentType },
    [UPVALUE]      = { "UpValue",     &UpValueType     },
    [VEC_NODE]     = { "VecNode",     &VecNodeType     },
    [VEC_LEAF]     = { "VecLeaf",     &VecLeafType     },
    [MAP_NODE]     = { "MapNode",     &MapNodeType     },
    [MAP_LEAF]     = { "MapLeaf",     &MapLeafType     },
    [BOTTOM]       = { "None",        &NoneType        },
    [TOP]          = { "Any",         &AnyType         },
    [TERM]         = { "Term",        &TermType        },
    [NUMBER]       = { "Number",      &NumberType      },
    [REAL]         = { "Real",        &RealType        },
    [RATIONAL]     = { "Rational",    &RationalType    },
    [INTEGER]      = { "Integer",     &IntegerType     },
  };

  for (size_t i=0;i <= INTEGER; i++) {
    char* name = types[i].name;
    Type* type = types[i].type;

    type->name       = getSymbol(name);
    type->obj.hash   = hashWord(type->idno);
    type->obj.hashed = true;

    defineGlobal(&RlVm, type->name, tag(&type), CONSTANTP);
  }
}
