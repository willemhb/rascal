#include "util/hashing.h"

#include "object.h"
#include "type.h"
#include "value.h"

// external API
Value tag_float(Float x) {
  return double_to_word(x);
}

Value tag_arity(Arity x) {
  return (x & VAL_MASK) | ARITY_TAG;
}

Value tag_small(Small x) {
  return ((Value)x) | SMALL_TAG;
}

Value tag_bool(Boolean x) {
  return x ? TRUE : FALSE;
}

Value tag_glyph(Glyph x) {
  return ((Value)x) | GLYPH_TAG;
}

Value tag_ptr(Pointer x) {
  return ((Value)x) | PTR_TAG;
}

Value tag_fptr(FuncPtr x) {
  return ((Value)x) | FPTR_TAG;
}

Value tag_obj(void* x) {
  return ((Value)x) | OBJ_TAG;
}

extern struct Type ArityType, SmallType, UnitType, BooleanType, GlyphType,
  PointerType, FuncPtrType, FloatType;

Type* type_of_val(Value value) {
  Type* out;
  
  switch (value & TAG_MASK) {
    case ARITY_TAG: out = &ArityType;                  break;
    case SMALL_TAG: out = &SmallType;                  break;
    case NUL_TAG:   out = &UnitType;                   break;
    case BOOL_TAG:  out = &BooleanType;                break;
    case GLYPH_TAG: out = &GlyphType;                  break;
    case OBJ_TAG:   out = type_of_obj(as_obj(value));  break;
    default:        out = &FloatType;                  break;
  }

  return out;
}

Type* type_of_obj(void* ptr) {
  assert(ptr != NULL);
  Obj*  obj = ptr;
  Type* out = obj->type;

  return out;
}

bool has_type_val(Value x, Type* type) {
  return is_instance(type_of(x), type);
}

bool has_type_obj(void* p, Type* type) {
  return is_instance(type_of(p), type);
}

// safecasts
#include "type.h"
#include "environment.h"
#include "runtime.h"

#define SAFECAST_1(T, t)                                                \
  extern struct Type T##Type;                                           \
                                                                        \
  T as_##t##_s(Value x, const char* fname) {                            \
    argtype(&T##Type, x, fname);                                        \
    return as_##t(x);                                                   \
  }

SAFECAST_1(Float, float);
SAFECAST_1(Arity, arity);
SAFECAST_1(Small, small);
SAFECAST_1(Boolean, bool);
SAFECAST_1(Glyph, glyph);
SAFECAST_1(Pointer, ptr);
SAFECAST_1(FuncPtr, fptr);


Obj* as_obj_s_val(Value x, const char* fname) {
  require(is_obj(x), "expected an object got a %s", fname, type_of(x)->name->name);
  return as_obj_s_obj(as_obj(x), fname);
}

Obj* as_obj_s_obj(void* p, const char* fname) {
  require(p != NULL, "unexpected null pointer reading object", fname);
  return p;
}

#define SAFECAST_2(T, t)                                                \
  extern struct Type T##Type;                                           \
                                                                        \
  T* as_##t##_s_val(Value x, const char* fname) {                       \
    require(x != NULL_OBJ, "unexpected NULL reading %s", fname, #T);    \
    argtype(&T##Type, x, fname);                                        \
    return as_##t(x);                                                   \
  }                                                                     \
                                                                        \
  T* as_##t##_s_obj(void* x, const char* fname) {                       \
    require(x != NULL, "unexpected NULL reading %s", fname, #T);        \
    require(is_##t(x),                                                  \
            "expected type was %s, actual was %s",                      \
            fname,                                                      \
            #T,                                                         \
            type_of(x)->name->name);                                    \
    return x;                                                           \
  }

SAFECAST_2(Symbol,      sym);
SAFECAST_2(Type,        type);
SAFECAST_2(Stream,      stream);
SAFECAST_2(Big,         big);
SAFECAST_2(Buffer8,     buf8);
SAFECAST_2(Buffer16,    buf16);
SAFECAST_2(Buffer32,    buf32);
SAFECAST_2(Binary8,     bin8);
SAFECAST_2(Binary16,    bin16);
SAFECAST_2(Binary32,    bin32);
SAFECAST_2(Alist,       alist);
SAFECAST_2(Objects,     objs);
SAFECAST_2(Table,       table);
SAFECAST_2(SymbolTable, symt);
SAFECAST_2(NameSpace,   ns);
SAFECAST_2(Bits,        bits);
SAFECAST_2(String,      str);
SAFECAST_2(Tuple,       tuple);
SAFECAST_2(List,        list);
SAFECAST_2(Vector,      vec);
SAFECAST_2(VecNode,     vecn);
SAFECAST_2(VecLeaf,     vecl);
SAFECAST_2(Map,         map);
SAFECAST_2(MapNode,     mapn);
SAFECAST_2(MapLeaf,     mapl);
SAFECAST_2(Function,    func);
SAFECAST_2(MethodTable, metht);
SAFECAST_2(MethodMap,   methm);
SAFECAST_2(Method,      method);
SAFECAST_2(Native,      native);
SAFECAST_2(Closure,     cls);
SAFECAST_2(Chunk,       chunk);
SAFECAST_2(Control,     cntl);
SAFECAST_2(Binding,     bind);
SAFECAST_2(Environment, envt);
SAFECAST_2(UpValue,     upval);
