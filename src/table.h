#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

#define ENTRY_SPEC(keyType, valType)		\
  keyType key;					\
  valType value


typedef struct
{
  ENTRY_SPEC( ObjString*, Value );
} Entry;

typedef struct
{
  ARRAY_SPEC( Entry );
} Table;

#endif
