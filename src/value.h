#ifndef rascal_value_h
#define rascal_value_h

#include "rascal.h"

/* C types */
#include "decl/alist.h"

ALIST(Values, Value);

typedef union {
  Value        asValue;
  Real         asReal;
  Small        asSmall;
  Bool         asBool;
  Glyph        asGlyph;
  Object       asObject;
  Symbol       asSymbol;
  Function     asFunction;
  List         asList;
  Pair         asPair;
  Tuple        asTuple;
  String       asString;
  ByteCode     asByteCode;
  NameSpc      asNameSpc;
  Environ      asEnviron;
  Method       asMethod;
  UserMethod   asUserMethod;
  NativeMethod asNativeMethod;
} ValueType;

/* globals */
#define QNAN    0x7ff8000000000000ul
#define RNAN    (nan(""))

#define REAL    0x0000000000000000ul
#define SMALL   0x7ffc000000000000ul
#define FIXNUM  0x7ffd000000000000ul
#define TYPE    0x7ffe000000000000ul
#define ERROR   0x7fff000000000000ul
#define UNIT    0xfffc000000000000ul
#define BOOL    0x7ffd000000000000ul
#define GLYPH   0xfffe000000000000ul
#define OBJECT  0xffff000000000000ul

#define TAGMASK 0xffff000000000000ul
#define PTRMASK (~TAGMASK)

#define NUL     UNIT
#define TRUE    (BOOL|1ul)
#define FALSE   (BOOL|0ul)

/* APIs */
RlType typeOf(Value x);
bool   hasType(Value x, RlType type);
bool   hasTag(Value x, RlTag tag);
void   printValue(Value x);
Value  realToValue(Real r);
Value  smallToValue(Small s);
Value  boolToValue(Bool b);
Value  glyphToValue(Glyph g);
Value  objectToValue(Object o);

/* convenience */
#define TAG_OF(x)     ((x)&TAGMASK)
#define DATA_OF(x)    ((x)&PTRMASK)
#define TAG_VAL(x, t) ((AS_VAL(x)&PTRMASK)|(t))

#define AS_VAL(x)     (((ValueType)(x)).asValue)
#define AS_REAL(x)    (((ValueType)(x)).asReal)
#define AS_SMALL(x)   (((ValueType)(x)).asSmall)
#define AS_BOOL(x)    ((x)==TRUE)
#define AS_GLYPH(x)   (((ValueType)(x)).asGlyph)
#define AS_OBJ(x)     ((Object)(AS_VAL(x)&PTRMASK))

#define IS_REAL(x)    hasType(x, RealType)
#define IS_SMALL(x)   hasTag(x, SMALL)
#define IS_NUL(x)     ((x)==NUL)
#define IS_BOOL(x)    hasTag(x, BOOL)
#define IS_GLYPH(x)   hasTag(x, GLYPH)
#define IS_OBJ(x)     hasTag(x, OBJECT)


#define INIT_OBJ(obj, init)						\
  do {									\
    usize headoff        =  sizeof(struct Object);			\
    struct Object *head  =  (struct Object*)((obj) - headoff);		\
    head->next           =  NULL;					\
    head->type           =  init->type;					\
    head->hash           =  init->hash;					\
    head->hashed         =  init->hashed;				\
    head->size           =  init->size;					\
    head->offset         =  init->offset;				\
    head->allocated      =  init->allocated;				\
    head->gray           =  init->allocated;				\
    head->black          = !init->allocated;				\
    head->inlined        =  init->inlined;				\
    head->lendee         =  init->lendee;				\
  } while (false)

#define VALUE(x)                                    \
  generic((x),                                      \
          Real:realToValue,                         \
          Small:smallToValue,                       \
          Bool:boolToValue,                         \
          Glyph:glyphToValue,                       \
          Object:objectToValue,                     \
          Symbol:symbolToValue,                     \
          Function:functionToValue,                 \
          List:listToValue,                         \
          Pair:pairToValue,                         \
          Tuple:tupleToValue,                       \
          String:stringToValue,                     \
          ByteCode:byteCodeToValue,                 \
          Environ:environToValue,                   \
          Method:methodToValue,                     \
          UserMethod:userMethodToValue,             \
          NativeMethod:nativeMethodToValue )(x)

#endif
