#ifndef rascal_hamt_h
#define rascal_hamt_h

#include "table.h"

// C types --------------------------------------------------------------------
struct HAMT
{
  OBJ_HEAD;
  Arity   bitmap;
  UInt16  height;
  UInt8   length;
  UInt8   capacity;
  Value  *data;
  Obj    *cache;
};

typedef struct HAMTPathBuffer HAMTPathBuffer;

// forward declarations -------------------------------------------------------
HAMT   *newHAMT( ObjType type );
void    initHAMT( HAMT *ob );
void    finalizeHAMT( HAMT *ob );
HAMT   *copyHAMT( HAMT *ob );

// 
Value  *vectorRef( Vector *ob, Arity index, HAMTPathBuffer *buffer );
Vector *vectorConj( Vector *ob, Value val, bool inPlace );
Value  *mapRef( Map *ob, Value key, HAMTPathBuffer *buffer );


// statics & utility macros ---------------------------------------------------
#define IS_VECTOR(val) (isObjType(val, OBJ_VECTOR))
#define IS_MAP(val)    (isObjType(val, OBJ_MAP))
#define IS_SET(val)    (isObjType(val, OBJ_SET))

#define AS_VECTOR(val) ((Vector*)AS_OBJ(val))
#define AS_MAP(val)    ((Map*)AS_OBJ(val))
#define AS_SET(val)    ((Set*)AS_OBJ(val))

#endif
