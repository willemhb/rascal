#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"
#include "array.h"

#define OBJ_TYPE(value)      (AS_OBJ(value)->type)
#define OBJ_PROTECTED(value) (AS_OBJ(value)->protected)
#define OBJ_GRAY(value)      (AS_OBJ(value)->gray)
#define OBJ_BLACK(value)     (AS_OBJ(value)->black)

#define IS_SYMBOL(value)   isObjType(value, OBJ_SYMBOL)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)
#define IS_LIST(value)     isObjType(value, OBJ_LIST)
#define IS_VECTOR(value)   isObjType(value, OBJ_VECTOR)

#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)
#define AS_SYMBOL(value)   ((ObjSymbol*)AS_OBJ(value))
#define AS_LIST(value)     ((ObjList*)AS_OBJ(value))
#define AS_VECTOR(value)   ((ObjVector*)AS_OBJ(value))

typedef enum
  {
    OBJ_SYMBOL=VAL_OBJ,
    OBJ_LIST,
    OBJ_STRING,
    OBJ_VECTOR,
    /*
      OBJ_DICT,
      OBJ_SET,
      OBJ_FUNCTION,
      OBJ_MACRO,
      OBJ_NATIVE,
      OBJ_CLOSURE,
      OBJ_BYTECODE,
     */
  } ObjType;

struct Obj
{
  Obj      *next;

  ObjType   type;
  uint32_t  flags     : 29;
  uint32_t  protected :  1;
  uint32_t  gray      :  1;
  uint32_t  black     :  1;
};

typedef enum
  {
    SYM_GENSYM =1,
    SYM_KEYWORD=2,
  } FlSymbol;

struct ObjSymbol
{
  HEADER;

  ObjString *name;
  uint64_t   hash;
};

typedef enum
  {
    STR_LATIN1=1,
    STR_ASCII,
    STR_UTF8,
    STR_UTF16,
    STR_UTF32
  } FlString;

struct ObjString
{
  HEADER;
  int  length;
  char *chars;
};

typedef enum
  {
    LIST_EXPRESSION=0x10000000,
  } FlList;

struct ObjList
{
  HEADER;
  int      length;
  int      line;       // only used if the list is an expression

  Value    head;
  ObjList *tail;
};

typedef enum
  {
    VEC_EXPRESSION=1,
  } FlVector;

struct ObjVector
{
  HEADER;
  ValueArray array;
};

// forward declarations -------------------------------------------------------
ObjString *copyString( const char *chars, int length );
ObjList   *makeList( Value head, ObjList *tail );
ObjVector *makeVector( void );

void printObject( Value value );

/// static helpers ------------------------------------------------------------
static inline bool isObjType( Value value, ObjType type )
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
