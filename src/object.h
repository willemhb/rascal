#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef struct Atom        Atom;
typedef struct List        List;
typedef struct Function    Function;
typedef struct Tuple       Tuple;
typedef struct HAMT        HAMT;
typedef struct String      String;
typedef struct Port        Port;

typedef struct HAMT        Vector;
typedef struct HAMT        Map;
typedef struct HAMT        Set;

typedef struct Table       Table;
typedef struct ArrayList   ArrayList;
typedef struct Closure     Closure;
typedef struct UpValue     UpValue;
typedef struct Native      Native;
typedef struct Bytecode    Bytecode;

typedef enum
{
  OBJ_ATOM=11,
  OBJ_LIST=12,
  OBJ_FUNCTION=13,
  OBJ_TUPLE=14,
  OBJ_VECTOR=15,
  OBJ_MAP=17,
  OBJ_SET=18,
  OBJ_STRING=19,
  OBJ_PORT=20,

  // internal types
  OBJ_TABLE=21,
  OBJ_ARRAY_LIST=22,
  OBJ_CLOSURE=23,
  OBJ_UPVALUE=25,
  OBJ_NATIVE=26,
  OBJ_BYTECODE=27,
  OBJ_MAP_ENTRY=28,
  OBJ_TABLE_ENTRY=29
} ObjType;

struct Obj
{
  Obj    *next;
  Arity   arity;
  UInt16  type;
  UInt16  flags : 13;
  UInt16  black :  1;
  UInt16  gray  :  1;
  UInt16  alloc :  1;
};

// forward declarations -------------------------------------------------------
void initObject( Obj *o, ObjType type );

// utility macros & statics ---------------------------------------------------
#define OBJ_HEAD \
  Obj object

#define ob_next(val)  (AS_OBJ(x)->next)
#define ob_arity(val) (AS_OBJ(x)->arity)
#define ob_type(val)  (AS_OBJ(x)->type)
#define ob_flags(val) (AS_OBJ(x)->flags)
#define ob_black(val) (AS_OBJ(x)->black)
#define ob_gray(val)  (AS_OBJ(x)->gray)
#define ob_alloc(val) (AS_OBJ(x)->alloc)

static inline bool isObjType( Value x, ObjType t )
{
  return IS_OBJ(x) && AS_OBJ(x)->type == t;
}

#endif
