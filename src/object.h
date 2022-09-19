#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// C types --------------------------------------------------------------------
struct Obj
{
  Obj      *next;
  ValueType type;
  UInt8               : 4;
  UInt8     singleton : 1;
  UInt8     alloc     : 1;
  UInt8     gray      : 1;
  UInt8     black     : 1;
  UInt16    flags;
  Arity     arity;
};

// globals --------------------------------------------------------------------
extern Obj  *Singletons[NUM_TYPES];
extern Value ValueTag[NUM_TYPES];
extern Bool  Immutable[NUM_TYPES];

// forward declarations and utility macros ------------------------------------
#define OBJ_HEAD(flagsType)			\
  struct					\
  {						\
    ValueType type;				\
    UInt8               : 4;			\
    UInt8     singleton : 1;			\
    UInt8     alloc     : 1;			\
    UInt8     gray      : 1;			\
    UInt8     black     : 1;			\
    flagsType flags;				\
    Arity     arity;				\
  } object

#define asObjType(obType, x)				\
  ((obType*)_Generic((x),				\
		    Value:(((Value)(x))&~POINTER),	\
		     default:((obType*)(x))))

#define asObj(x)      asObjType(Obj, x)
#define asFunction(x) asObjType(Function, x)
#define asBinary(x)   asObjType(Binary, x)
#define asVector(x)   asObjType(Vector, x)
#define asMap(x)      asObjType(Map, x)
#define asClosure(x)  asObjType(Closure, x)

#define objNext(val)      (asObj(val)->next)
#define objType(val)      (asObj(val)->type)
#define objSingleton(val) (asObj(val)->singleton)
#define objAlloc(val)     (asObj(val)->alloc)
#define objGray(val)      (asObj(val)->gray)
#define objBlack(val)     (asObj(val)->black)
#define objFlags(val)     (asObj(val)->flags)
#define objArity(val)     (asObj(val)->arity)

#define NIL_OBJ  (Singletons[VAL_LIST])
#define NIL_LIST ((List*)NIL_OBJ)

#define VEC0_OBJ (Singletons[VAL_VECTOR])
#define VEC0_VEC ((Vector*)VEC0_OBJ)

// forward declarations -------------------------------------------------------
Void initObj( Obj *object, ValueType type, Flags fl );

// static & utilities ---------------------------------------------------------
static inline Bool isObjType( Value x, ValueType type )
{
  return isObj(x)
    && objType( x ) == type;
}

static inline Value tagObj( Obj *o )
{
  if (objSingleton(o))
    return IMMEDIATE|objType(o);

  return tagValue(o, OBJECT);
}

#endif
