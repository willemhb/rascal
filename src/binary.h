#ifndef rascal_binary_h
#define rascal_binary_h

#include "object.h"
#include "number.h"

// C types --------------------------------------------------------------------
typedef struct
{
  CType Ctype;
  UInt8 inlined   : 4;
  UInt8 shared    : 4;
} BinaryFl;

struct String
{
  OBJ_HEAD(BinaryFl);

  Char *chars;
  Hash  hash;
};

struct CodeBuffer
{
  OBJ_HEAD(BinaryFl);

  Instruction *code;
  Arity length, capacity;
};

// forward declarations


// utility macros
#define isString(val)   isObjType(val, VAL_STRING)
#define asString(val)   asObjType(String, val)

#define strCtype(val)   (asString(val)->flags.Ctype)
#define strInlined(val) (asString(val)->flags.inlined)
#define strShared(val)  (asString(val)->flags.shared)
#define strChars(val)   (asString(val)->chars)

#endif
