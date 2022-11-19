#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// core object types are defined here
typedef struct Cons
{
  OBJECT
  Value car;
  Value cdr;
} Cons;

typedef struct Type
{
  OBJECT

  Char *name;
  Size  size;
} Type;

// globals
extern Type TypeType, ConsType;

// forward declarations
Value cons( Value car, Value cdr );

#endif
