#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

struct Obj
{
  Obj      *next;
  ValueType type;
  UInt8     alloc : 1;
  UInt8     gray  : 1;
  UInt8     black : 1;
  UInt8           : 5;
  UInt16    flags;
  Arity     arity;
};

#define OBJ_HEAD(flagsType)			\
  struct					\
  {						\
    ValueType type;				\
    UInt8     Ctype : 5;			\
    UInt8     alloc : 1;			\
    UInt8     gray  : 1;			\
    UInt8     black : 1;			\
    flagsType flags;				\
    Arity     arity;				\
  } object

#define objNext(val)  (asObj(val)->next)
#define objType(val)  (asObj(val)->type)
#define objCtype(val) (asObj(val)->Ctype)
#define objAlloc(val) (asObj(val)->alloc)
#define objGray(val)  (asObj(val)->gray)
#define objBlack(val) (asObj(val)->black)
#define objFlags(val) (asObj(val)->flags)
#define objArity(val) (asObj(val)->arity)

static inline Bool isObj( Value x )
{
  return !isImmediate(x)
    && asPtr(x) != NULL;
}

static inline Bool isObjType( Value x, ValueType type )
{
  return isObj(x)
    && objType( x ) == type;
}

static inline Value tagObj( Obj *o )
{
  switch( o->type )
    {
    case VAL_LIST: case VAL_UPVALUE:
      return tagPtr( o, LIST );

    case VAL_MAP:
      return tagPtr( o, MAP );

    case VAL_VECTOR:
      return tagPtr( o, VECTOR );

    case VAL_FUNCTION:
      return tagPtr( o, FUNCTION );

    case VAL_CLOSURE:
      return tagPtr( o, CLOSURE );

    default:
      return tagPtr( o, BINARY );
    }
}

#endif
