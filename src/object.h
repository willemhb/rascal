#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef struct List        List;
typedef struct Atom        Atom;
typedef struct Function    Function;
typedef struct Tuple       Tuple;
typedef struct Map         Map;
typedef struct Set         Set;
typedef struct String      String;
typedef struct Bytecode    Bytecode;
typedef struct Template    Template;
typedef struct Closure     Closure;
typedef struct Native      Native;

typedef struct Environment Environment;
typedef struct UpValue     UpValue;

typedef enum
{
  OBJ_ATOM=1,
  OBJ_LIST=2,
  OBJ_FUNCTION=3,
  OBJ_TUPLE=4,
  OBJ_MAP=5,
  OBJ_SET=6,
  OBJ_STRING=7,

  OBJ_BYTECODE=9,
  OBJ_TEMPLATE=10,
  OBJ_CLOSURE=11,
  OBJ_NATIVE=12,
  OBJ_ENVIRONMENT=13,
  OBJ_UPVALUE=14,
  
  OBJ_PORT=15,
} ObjType;

struct Obj
{
  Value   next;   // tagged free list. Tag stores GC tags and allocation tag
  ObjType type;   // self explanatory
  int     arity;  // no fixed meaning
};

// forward declarations -------------------------------------------------------
void initObject( Obj *o, ObjType type );

// utility macros & statics ---------------------------------------------------
#define OBJ_HEAD \
  Obj object

static inline bool isObjType( Value x, ObjType t )
{
  return IS_OBJ(x) && AS_OBJ(x)->type == t;
}

static inline bool isNilObjType( Value x, ObjType t )
{
  return x == NIL || isObjType( x, t );
}

#endif
