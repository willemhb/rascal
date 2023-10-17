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

#define tag(x) _Generic((x),                    \
                        Float:tagFloat,         \
                        Arity:tagArity,         \
                        Small:tagSmall,         \
                        Boolean:tagBoolean,     \
                        Glyph:tagGlyph,         \
                        void*:tagPtr,           \
                        funcptr_t:tagFptr,      \
                        default:tagObj)(x)

#define AS_FLOAT(x)        ((Float)wordToDouble(x))
#define AS_ARITY(x)        ((x) & VAL_MASK)
#define AS_SMALL(x)        ((Small)((x) & SMALL_MASK))
#define AS_BOOL(x)         ((x) == TRUE)
#define AS_GLYPH(x)        ((Glyph)((x) & SMALL_MASK))
#define AS_PTR(x)          ((Pointer)((x) & VAL_MASK))
#define AS_FPTR(x)         ((FuncPtr)((x) & VAL_MASK))
#define AS(T, x)           ((T*)AS_PTR(x))

#define IS_OBJ(x)          (((x) & TAG_MASK) == OBJ_TAG)
#define IS(T, x)           hasType(x, &T##Type)

#define typeOf(v)         generic2(typeOf, v, v)
#define sizeOf(v)         generic2(sizeOf, v, v)
#define hasType(v, t)     generic2(hasType, v, v, t)

Value tagFloat(Float x);
Value tagArity(Arity x);
Value tagSmall(Small x);
Value tagBoolean(Boolean x);
Value tagGlyph(Glyph x);
Value tagPtr(Pointer x);
Value tagFptr(FuncPtr x);
Value tagObj(void* x);

Type*  typeOfVal(Value x);
Type*  typeOfObj(void* p);
size_t sizeOfVal(Value x);
size_t sizeOfObj(void* p);
bool   hasTypeVal(Value x, Type* type);
bool   hasTypeObj(void* p, Type* type);

#endif
