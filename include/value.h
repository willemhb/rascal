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

#define tag(x) _Generic((x),                     \
                        Float:tag_float,         \
                        Arity:tag_arity,         \
                        Small:tag_small,         \
                        Boolean:tag_bool,        \
                        Glyph:tag_glyph,         \
                        Pointer:tag_ptr,         \
                        FuncPtr:tag_fptr,        \
                        default:tag_obj)(x)

#define as_float(x) ((Float)word_to_double(x))
#define as_arity(x) ((x) & VAL_MASK)
#define as_small(x) ((Small)((x) & SMALL_MASK))
#define as_bool(x)  ((x) == TRUE)
#define as_glyph(x) ((Glyph)((x) & SMALL_MASK))
#define as_ptr(x)   ((Pointer)((x) & VAL_MASK))
#define as_fptr(x)  ((FuncPtr)((x) & VAL_MASK))
#define as(T, x)    ((T*)as_ptr(x))

#define is_obj(x)   (((x) & TAG_MASK) == OBJ_TAG)
#define is(T, x)    hasType(x, &T##Type)

#define type_of(v)     generic2(type_of, v, v)
#define size_of(v)     generic2(size_of, v, v)
#define has_type(v, t) generic2(has_type, v, v, t)

Value tag_float(Float x);
Value tag_arity(Arity x);
Value tag_small(Small x);
Value tag_bool(Boolean x);
Value tag_glyph(Glyph x);
Value tag_ptr(Pointer x);
Value tag_fptr(FuncPtr x);
Value tag_obj(void* x);

Type*  type_of_val(Value x);
Type*  type_of_obj(void* p);
size_t size_of_val(Value x);
size_t size_of_obj(void* p);
bool   has_type_val(Value x, Type* type);
bool   has_type_obj(void* p, Type* type);

#endif
