#include <string.h>

#include "value.h"
#include "object.h"
#include "metaobject.h"
#include "product.h"
#include "text.h"
#include "number.h"

#include "hashing.h"

// globals --------------------------------------------------------------------
type_t* BuiltinTypes[NUM_TYPES];

// API ------------------------------------------------------------------------
// tags, tagging, types, queries ----------------------------------------------
val_type_t val_type(value_t val) {
  switch (val & TAG_MASK) {
    case SMALLTAG: return val >> 32 & 0x3f;
    case OBJTAG:   return OBJECT;
    case FIXTAG:   return FIXNUM;
    case PTRTAG:   return POINTER;
    default:       return REAL;
  }
}

obj_type_t val_obj_type(value_t val) {
  assert((val & TAG_MASK) == OBJTAG);

  return obj_obj_type(as_object(val));
}

obj_type_t obj_obj_type(void* ptr) {
  assert(ptr);
  return as_object(ptr)->type;
}

data_type_t* val_type_of(value_t val) {
  val_type_t vt = val_type(val);

  if (vt == OBJECT)
    return obj_type_of(as_object(val));

  return (data_type_t*)BuiltinTypes[vt];
}

data_type_t* obj_type_of(void* ptr) {
  object_t* obj = ptr;
  assert(obj);

  if (obj->type == STRUCT || obj->type == RECORD)
    return ((record_t*)obj)->type;

  return (data_type_t*)BuiltinTypes[obj->type];
}

// core APIs ------------------------------------------------------------------
kind_t val_has_type(value_t val, type_t* type) {
  if (type->isa)
    return type->isa(val, type);

  return type_of(val) == (data_type_t*)type ? DATA_KIND : BOTTOM_KIND;
}

kind_t obj_has_type(void* ptr, type_t* type) {
  object_t* obj = ptr;
  assert(obj);
  return val_has_type(object(obj), type);
}

void val_print(value_t val, port_t* ios) {
  data_type_t* type = type_of(val);

  if (type->print)
    type->print(val, ios);

  else
    rl_printf(ios, "#%s<%.48ul>", type->type.name, val & VAL_MASK);
}

void obj_print(void* ptr, port_t* ios) {
  assert(ptr);
  val_print(object(ptr), ios);
}

usize val_size_of(value_t val) {
  if (is_object(val))
    return obj_size_of(as_object(val));
  
  data_type_t* type = (data_type_t*)type_of(val);

  return type->size;
}

usize obj_size_of(void* ptr) {
  object_t* obj = ptr;
  assert(obj);

  data_type_t* type = type_of(obj);

  usize out = type->size;

  if (type->size_of)
    out += type->size_of(obj);

  return out;
}


uhash val_hash(value_t x) {
  if (is_object(x))
    return obj_hash(as_object(x));

  return hash_uword(x);
}

uhash obj_hash(void* ptr) {
  object_t* obj = ptr;
  assert(obj);

  if (obj->hashed)
    return obj->hash;

  data_type_t* type = type_of(obj);

  if (type->hash) {
    obj->hash   = type->hash(obj);
    obj->hashed = true;

    return obj->hash;
  }

  return hash_uword(object(obj));
}

bool val_equal(value_t x, value_t y) {
  if (x == y)
    return true;

  if (is_object(x)) {
    if (!is_object(y))
      return false;

    return obj_equal(as_object(x), as_object(y));
  }

  return false;
}

bool obj_equal(void* px, void* py) {
  object_t* x = px, * y = py;
  assert(x);
  assert(y);

  if (x == y)
    return true;

  data_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return false;

  if (xtype->equal)
    return xtype->equal(x, y);

  return false;
}

int val_compare(value_t x, value_t y) {
  if (x == y)
    return 0;

  data_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return CMP(xtype->type.idno, ytype->type.idno);

  if (xtype->compare)
    return xtype->compare(x, y);

  return CMP(x, y);
}

int obj_compare(void* px, void* py) {
  object_t* x = px, * y = py;
  assert(x);
  assert(y);

  if (x == y)
    return 0;

  data_type_t* xtype = type_of(x), * ytype = type_of(y);

  if (xtype != ytype)
    return CMP(xtype->type.idno, ytype->type.idno);

  if (xtype->compare)
    return xtype->compare(object(x), object(y));

  usize xsize = rl_size_of(x), ysize = rl_size_of(y), maxc = MIN(xsize, ysize);
  int o;

  if ((o=memcmp(x, y, maxc)))
    return o;

  return 0 - (xsize < ysize) + (ysize > xsize);
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
  extern data_type_t DataTypeType, UnionTypeType;

  BuiltinTypes[DATA_TYPE]  = &DataTypeType.type;
  BuiltinTypes[UNION_TYPE] = &UnionTypeType.type;

  extern data_type_t ArrNodeType,
    MapNodeType, MapLeafType, MapLeavesType,
    MethodTableType, MethodType,
    ChunkType, ClosureType,
    VariableType, NamespaceType, EnvironmentType,
    ControlType;

  BuiltinTypes[ARR_NODE] = &ArrNodeType.type;
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

  extern data_type_t SymbolType, FunctionType, PortType,
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

  extern data_type_t Sint8Type, Uint8Type, Sint16Type, Uint16Type,
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
  for (int i=DATA_TYPE; i<NUM_TYPES; i++) {
    if (i == OBJECT || i == RECORD || i == STRUCT)
      continue;

    rl_hash(&BuiltinTypes[i]->obj);
  }
}
