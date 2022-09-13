#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)   (AS_OBJ(value)->type)

#define IS_SYMBOL(value)  isObjType(value, OBJ_SYMBOL)
#define IS_STRING(value)  isObjType(value, OBJ_STRING)
#define IS_LIST(value)    isObjType(value, OBJ_LIST)
#define IS_VECTOR(value)  isObjType(value, OBJ_VECTOR)

#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) ((ObjString*)AS_OBJ(value)->chars)

typedef enum
  {
    OBJ_SYMBOL=VAL_OBJ,
    OBJ_LIST,
    OBJ_STRING,
    OBJ_VECTOR,
  } ObjType;

struct Obj
{
  ObjType type;
};

struct ObjSymbol
{
  HEADER;
  bool isGensym;
  bool isKeyword;
  bool isConstant;

  ObjString *name;
  uint64_t   hash;
};

struct ObjString
{
  HEADER;

  int  length;
  char *chars;
};

struct ObjList
{
  HEADER;
  int length;
  Value head, tail;
};

struct ObjVector
{
  HEADER;
  ARRAY_HEADER;
  Value *values;
};

// forward declarations -------------------------------------------------------

static inline bool isObjType( Value value, ObjType type )
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
