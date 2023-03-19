#include "value.h"
#include "metaobject.h"
#include "product.h"
#include "text.h"
#include "hashing.h"
#include "number.h"

// globals --------------------------------------------------------------------
rl_type_t* BuiltinTypes[NUM_TYPES];

// API ------------------------------------------------------------------------
// tags, tagging, types, queries ----------------------------------------------
val_type_t val_type(value_t val) {
  switch (val & TAG_MASK) {
    case SMALLTAG: return val >> 32 & 0x3f;
    case FIXTAG:   return FIXNUM;
    case NULTAG:   return UNIT;
    case BOOLTAG:  return BOOLEAN;
    case PTRTAG:   return POINTER;
    case OBJTAG:   return OBJECT;
    default:       return REAL;
  }
}

obj_type_t val_obj_type(value_t val) {
  assert((val & TAG_MASK) == OBJTAG);

  return obj_obj_type(as_obj(val));
}

obj_type_t obj_obj_type(object_t* obj) {
  assert(obj);
  return obj->type;
}

rl_type_t* val_type_of(value_t val) {
  val_type_t vt = val_type(val);

  if (vt == OBJECT)
    return obj_type_of(as_obj(val));

  return BuiltinTypes[vt];
}

rl_type_t* obj_type_of(object_t* obj) {
  assert(obj);

  if (obj->type == STRUCT || obj->type == RECORD)
    return (rl_type_t*)((record_t*)obj)->type;

  return BuiltinTypes[obj->type];
}

// core APIs ------------------------------------------------------------------
kind_t val_has_type(value_t val, rl_type_t* type) {
  if (type->isa)
    return type->isa(val, type);

  return type_of(val) == type && DATA_KIND;
}

kind_t obj_has_type(object_t* obj, rl_type_t* type) {
  assert(obj);
  return val_has_type(object(obj), type);
}

void val_print(value_t val, port_t* ios) {
  rl_type_t* type = type_of(val);

  if (type->vtable->print)
    type->vtable->print(val, ios);

  else
    rl_printf(ios, "#%s<%.48ul>", type->name, val & VAL_MASK);
}

void obj_print(object_t* obj, port_t* ios) {
  val_print(object(obj), ios);
}

usize val_size_of(value_t val) {
  if (is_obj(val))
    return obj_size_of(as_obj(val));
  
  primitive_type_t* type = (primitive_type_t*)type_of(val);

  return type->size;
}

usize obj_size_of(object_t* obj) {
  assert(obj);

  object_type_t* type = (object_type_t*)type_of(obj);

  usize out = type->size;

  if (type->type.vtable->size)
    out += type->type.vtable->size(obj);

  return out;
}


uhash val_hash(value_t x) {
  if (is_obj(x))
    return obj_hash(as_obj(x));

  return hash_uword(x);
}

uhash obj_hash(object_t* obj) {
  assert(obj);

  if (obj->hashed)
    return obj->hash;

  rl_type_t* type = type_of(obj);

  if (type->vtable->hash) {
    obj->hash   = type->vtable->hash(obj);
    obj->hashed = true;

    return obj->hash;
  }

  return hash_uword(object(obj));
}

bool val_equal(value_t x, value_t y) {
  if (x == y)
    return true;

  if (is_obj(x)) {
    if (!is_obj(y))
      return false;

    return obj_equal(as_obj(x), as_obj(y));
  }

  return false;
}

bool obj_equal(object_t* x, object_t* y) {
  assert(x);
  assert(y);

  if (x == y)
    return true;

  rl_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return false;

  if (xtype->vtable->equal)
    return xtype->vtable->equal(x, y);

  return false;
}

int val_compare(value_t x, value_t y) {
  if (x == y)
    return 0;

  rl_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return CMP(xtype->idno, ytype->idno);

  if (xtype->vtable->compare)
    return xtype->vtable->compare(x, y);

  return CMP(x, y);
}

int obj_compare(object_t* x, object_t* y) {
  assert(x);
  assert(y);

  if (x == y)
    return 0;

  rl_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return CMP(xtype->idno, ytype->idno);

  if (xtype->vtable->compare)
    return xtype->vtable->compare(object(x), object(y));

  return CMP(object(x), object(y));
}

// flag helpers ---------------------------------------------------------------
bool has_flag(void* ptr, flags fl) {
  assert(ptr);
  object_t* obj = ptr;
  return !!(obj->flags & fl);
}

bool set_flag(void* ptr, flags fl) {
  assert(ptr);
  object_t* obj = ptr;
  bool has = !!(obj->flags & fl);
  obj->flags |= fl;
  return !has;
}

bool clear_flag(void* ptr, flags fl) {
  assert(ptr);
  object_t* obj = ptr;
  bool has = !!(obj->flags & fl);
  obj->flags &= ~fl;
  return has;
}

// initialization -------------------------------------------------------------
void value_init(void) {
  extern object_type_t PrimitiveTypeType, ObjectTypeType, UnionTypeType;

  BuiltinTypes[PRIMITIVE_TYPE] = &PrimitiveTypeType.type;
  BuiltinTypes[OBJECT_TYPE] = &ObjectTypeType.type;
  BuiltinTypes[UNION_TYPE] = &UnionTypeType.type;

  extern object_type_t ArrNodeType, ArrLeafType,
    MapNodeType, MapLeafType, MapLeavesType,
    MethodTableType, MethodType,
    ChunkType, ClosureType,
    VariableType, NamespaceType, EnvironmentType,
    ControlType;

  BuiltinTypes[ARR_NODE] = &ArrNodeType.type;
  BuiltinTypes[ARR_LEAF] = &ArrLeafType.type;
  BuiltinTypes[MAP_NODE] = &MapNodeType.type;
  BuiltinTypes[MAP_LEAF] = &MapLeafType.type;
  BuiltinTypes[MAP_LEAVES] = &MapLeavesType.type;
  BuiltinTypes[METHOD_TABLE] = &MethodTableType.type;
  BuiltinTypes[METHOD] = &MethodType.type;
  BuiltinTypes[CHUNK] = &ChunkType.type;
  BuiltinTypes[CLOSURE] = &ClosureType.type;
  BuiltinTypes[VARIABLE] = &VariableType.type;
  BuiltinTypes[NAMESPACE] = &NamespaceType.type;
  BuiltinTypes[ENVIRONMENT] = &EnvironmentType.type;
  BuiltinTypes[CONTROL] = &ControlType.type;

  extern object_type_t SymbolType, FunctionType, PortType,
    BinaryType, StringType, TupleType, ListType, VectorType,
    DictType, SetType, TableType, AlistType, BufferType,
    ComplexType, RatioType, BigType;

  BuiltinTypes[SYMBOL] = &SymbolType.type;
  BuiltinTypes[FUNCTION] = &FunctionType.type;
  BuiltinTypes[PORT] = &PortType.type;
  BuiltinTypes[BINARY] = &BinaryType.type;
  BuiltinTypes[STRING] = &StringType.type;
  BuiltinTypes[TUPLE] = &TupleType.type;
  BuiltinTypes[LIST] = &ListType.type;
  BuiltinTypes[VECTOR] = &VectorType.type;
  BuiltinTypes[DICT] = &DictType.type;
  BuiltinTypes[SET] = &SetType.type;
  BuiltinTypes[TABLE] = &TableType.type;
  BuiltinTypes[ALIST] = &AlistType.type;
  BuiltinTypes[BUFFER] = &BufferType.type;
  BuiltinTypes[COMPLEX] = &ComplexType.type;
  BuiltinTypes[RATIO] = &RatioType.type;
  BuiltinTypes[BIG] = &BigType.type;

  extern primitive_type_t Sint8Type, Uint8Type, Sint16Type, Uint16Type,
    Sint32Type, Uint32Type, Real32Type, FixnumType, RealType,
    AsciiType, Latin1Type, Utf8Type, Utf16Type, Utf32Type,
    BooleanType, PointerType, UnitType;

  BuiltinTypes[SINT8] = &Sint8Type.type;
  BuiltinTypes[UINT8] = &Uint8Type.type;
  BuiltinTypes[SINT16] = &Sint16Type.type;
  BuiltinTypes[UINT16] = &Uint16Type.type;
  BuiltinTypes[SINT32] = &Sint32Type.type;
  BuiltinTypes[UINT32] = &Uint32Type.type;
  BuiltinTypes[REAL32] = &Real32Type.type;
  BuiltinTypes[FIXNUM] = &FixnumType.type;
  BuiltinTypes[REAL] = &RealType.type;
  BuiltinTypes[ASCII] = &AsciiType.type;
  BuiltinTypes[LATIN1] = &Latin1Type.type;
  BuiltinTypes[UTF8] = &Utf8Type.type;
  BuiltinTypes[UTF16] = &Utf16Type.type;
  BuiltinTypes[UTF32] = &Utf32Type.type;
  BuiltinTypes[BOOLEAN] = &BooleanType.type;
  BuiltinTypes[POINTER] = &PointerType.type;
  BuiltinTypes[UNIT] = &UnitType.type;

  extern union_type_t NoneType, AnyType;

  BuiltinTypes[NONE] = &NoneType.type;
  BuiltinTypes[ANY] = &AnyType.type;

  // initialize type hashes ---------------------------------------------------
  for (int i=PRIMITIVE_TYPE; i<NUM_TYPES; i++) {
    if (i == OBJECT || i == RECORD || i == STRUCT)
      continue;

    rl_hash(&BuiltinTypes[i]->obj);
  }
}
