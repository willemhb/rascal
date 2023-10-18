#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// tags and such
#define ARITY_TAG   0x7ffc000000000000UL
#define SMALL_TAG   0x7ffd000000000000UL
#define BOOL_TAG    0x7ffe000000000000UL
#define NUL_TAG     0x7fff000000000000UL
#define GLYPH_TAG   0xfffc000000000000UL
#define PTR_TAG     0xfffd000000000000UL
#define FPTR_TAG    0xfffe000000000000UL
#define OBJ_TAG     0xffff000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL
#define SMALL_MASK  0x00000000ffffffffUL

#define TRUE        (BOOL_TAG  | 1UL)
#define FALSE       (BOOL_TAG  | 0UL)
#define NUL         (NUL_TAG   | 0UL)
#define NOTHING     (NUL_TAG   | 1UL) // invalid value marker
#define ZERO        (SMALL_TAG | 0UL)
#define ONE         (SMALL_TAG | 1UL)

#define tag(x) generic((x),                      \
                       Float:tag_float,          \
                       Arity:tag_arity,          \
                       Small:tag_small,          \
                       Boolean:tag_bool,         \
                       Glyph:tag_glyph,          \
                       Pointer:tag_ptr,          \
                       FuncPtr:tag_fptr,         \
                       Obj*:tag_obj,             \
                       Symbol*:tag_obj,          \
                       Type*:tag_obj,            \
                       Stream*:tag_obj,          \
                       Big*:tag_obj,             \
                       Buffer8*:tag_obj,         \
                       Buffer16*:tag_obj,        \
                       Buffer32*:tag_obj,        \
                       Binary8*:tag_obj,         \
                       Binary16*:tag_obj,        \
                       Binary32*:tag_obj,        \
                       Alist*:tag_obj,           \
                       Objects*:tag_obj,         \
                       Table*:tag_obj,           \
                       SymbolTable*:tag_obj,     \
                       NameSpace*:tag_obj,       \
                       Bits*:tag_obj,            \
                       String*:tag_obj,          \
                       Tuple*:tag_obj,           \
                       List*:tag_obj,            \
                       Vector*:tag_obj,          \
                       VecNode*:tag_obj,         \
                       VecLeaf*:tag_obj,         \
                       Map*:tag_obj,             \
                       MapNode*:tag_obj,         \
                       MapLeaf*:tag_obj,         \
                       Function*:tag_obj,        \
                       MethodTable*:tag_obj,     \
                       MethodMap*:tag_obj,       \
                       MethodNode*:tag_obj,      \
                       Native*:tag_obj,          \
                       Closure*:tag_obj,         \
                       Chunk*:tag_obj,           \
                       Control*:tag_obj,         \
                       Binding*:tag_obj,         \
                       Scope*:tag_obj,           \
                       Environment*:tag_obj,     \
                       UpValue*:tag_obj,         \
                       default:tag_ptr)(x)

#define untag(x)      ((x) & VAL_MASK)

#define as(T, x, c)      ((T)c(x))
#define as2(T, x, c)                                            \
  generic((x),                                                  \
          Value:as(T, (Value)(x), c),                           \
          default:((T)(x)))

#define as_float(x)        as2(Float, x, word_to_double)
#define as_arity(x)        as2(Arity, x, untag)
#define as_small(x)        as2(Small, x, untag)
#define as_bool(x)         as2(Bool, x, untag)
#define as_glyph(x)        as2(Glyph, x, untag)
#define as_ptr(x)          as2(Pointer, x, untag)
#define as_fptr(x)         as2(FuncPtr, x, untag)
#define as_obj(x)          as2(Obj*, x, untag)
#define as_type(x)         as2(Type*, x, untag)
#define as_stream(x)       as2(Stream*, x, untag)
#define as_big(x)          as2(Big*, x, untag)
#define as_buf8(x)         as2(Buffer8*, x, untag)
#define as_buf16(x)        as2(Buffer16*, x, untag)
#define as_buf32(x)        as2(Buffer32*, x, untag)
#define as_bin8(x)         as2(Binary8*, x, untag)
#define as_bin16(x)        as2(Binary16*, x, untag)
#define as_bin32(x)        as2(Binary32*, x, untag)
#define as_alist(x)        as2(Alist*, x, untag)
#define as_objs(x)         as2(Objects*, x, untag)
#define as_table(x)        as2(Table*, x, untag)
#define as_ns(x)           as2(NameSpace*, x, untag)
#define as_symt(x)         as2(SymbolTable*, x, untag)
#define as_bits(x)         as2(Bits*, x, untag)
#define as_str(x)          as2(String*, x, untag)
#define as_tuple(x)        as2(Tuple*, x, untag)
#define as_list(x)         as2(List*, x, untag)
#define as_vec(x)          as2(Vector*, x, untag)
#define as_vecn(x)         as2(VecNode*, x, untag)
#define as_vecl(x)         as2(VecLeaf*, x, untag)
#define as_map(x)          as2(Map*, x, untag)
#define as_mapn(x)         as2(MapNode*, x, untag)
#define as_mapl(x)         as2(MapLeaf*, x, untag)
#define as_func(x)         as2(Function*, x, untag)
#define as_metht(x)        as2(MethodTable*, x, untag)
#define as_methm(x)        as2(MethodMap*, x, untag)
#define as_methn(x)        as2(MethodNode*, x, untag)
#define as_native(x)       as2(Native*, x, untag)
#define as_cls(x)          as2(Closure*, x, untag)
#define as_chunk(x)        as2(Chunk*, x, untag)
#define as_cntl(x)         as2(Control*, x, untag)
#define as_sym(x)          as2(Symbol*, x, untag)
#define as_bind(x)         as2(Binding*, x, untag)
#define as_envt(x)         as2(Environment*, x, untag)
#define as_upval(x)        as2(UpValue*, x, untag)

#define is(T, x)        has_type(x, &T##Type)
#define is_obj(x)       (((x) & TAG_MASK) == OBJ_TAG)
#define is_float(x)     is(Float, x)
#define is_arity(x)     is(Arity, x)
#define is_small(x)     is(Small, x)
#define is_bool(x)      is(Boolean, x)
#define is_unit(x)      is(Unit, x)
#define is_glyph(x)     is(Glyph, x)
#define is_ptr(x)       is(Pointer, x)
#define is_fptr(x)      is(FuncPtr, x)
#define is_sym(x)       is(Symbol, x)
#define is_type(x)      is(Type, x)
#define is_stream(x)    is(Stream, x)
#define is_big(x)       is(Big, x)
#define is_buf8(x)      is(Buffer8, x)
#define is_buf16(x)     is(Buffer16, x)
#define is_buf32(x)     is(Buffer32, x)
#define is_bin8(x)      is(Binary8, x)
#define is_bin16(x)     is(Binary16, x)
#define is_bin32(x)     is(Binary32, x)
#define is_alist(x)     is(Alist, x)
#define is_objs(x)      is(Objects, x)
#define is_table(x)     is(Table, x)
#define is_symt(x)      is(SymbolTable, x)
#define is_ns(x)        is(NameSpace, x)
#define is_bits(x)      is(Bits, x)
#define is_str(x)       is(String, x)
#define is_tuple(x)     is(Tuple, x)
#define is_list(x)      is(List, x)
#define is_vec(x)       is(Vector, x)
#define is_vecn(x)      is(VecNode, x)
#define is_vecl(x)      is(VecLeaf, x)
#define is_map(x)       is(Map, x)
#define is_mapn(x)      is(MapNode, x)
#define is_mapl(x)      is(MapLeaf, x)
#define is_func(x)      is(Function, x)
#define is_metht(x)     is(MethodTable, x)
#define is_methm(x)     is(MethodMap, x)
#define is_methn(x)     is(MethodNode, x)
#define is_method(x)    is(Method, x)
#define is_native(x)    is(Native, x)
#define is_cls(x)       is(Closure, x)
#define is_chunk(x)     is(Chunk, x)
#define is_cntl(x)      is(Control, x)
#define is_bind(x)      is(Binding, x)
#define is_envt(x)      is(Environment, x)
#define is_upval(x)     is(UpValue, x)



#define type_of(v)     generic2(type_of, v, v)
#define size_of(v)     generic2(size_of, v, v)
#define has_type(v, T) generic2(has_type, v, v, T)


Value tag_float(Float x);
Value tag_arity(Arity x);
Value tag_small(Small x);
Value tag_bool(Boolean x);
Value tag_glyph(Glyph x);
Value tag_ptr(Pointer x);
Value tag_fptr(FuncPtr x);
Value tag_obj(void* x);

#define SAFECAST_V(T, t)                        \
  T to_##t(Value x, const char* fname)

SAFECAST_V(Float, float);
SAFECAST_V(Arity, arity);
SAFECAST_V(Small, small);
SAFECAST_V(Boolean, bool);
SAFECAST_V(Glyph, glyph);
SAFECAST_V()

#define SAFECAST_O(T, t)                        \
  T to_##t(void* p, const char* fname)


Type*  type_of_val(Value x);
Type*  type_of_obj(void* p);
size_t size_of_val(Value x);
size_t size_of_obj(void* p);
bool   has_type_val(Value x, Type* type);
bool   has_type_obj(void* p, Type* type);

#endif
