
#ifndef rascal_table_h
#define rascal_table_h

#include "binary.h"

// parameters -----------------------------------------------------------------
// C types --------------------------------------------------------------------
struct ArrayList
{
  OBJ_HEAD(BinaryFl);
  Arity  length;
  Arity  cap;
  Value *values;
};

typedef struct
{
  OBJ_HEAD(BinaryFl);
  Char *chars;
  Hash  hash;
  Atom *value;
} Entry;

struct Table
{
  OBJ_HEAD(BinaryFl);
  Idno    counter;
  Arity   length;
  Arity   capacity;
  Entry **entries;
};

// forward declarations -------------------------------------------------------
// hashing utilities ----------------------------------------------------------
Hash hashCstring( const Char *cstr );
Hash hashMemory( const UInt8 *bytes, Arity nBytes );
Hash hashInt( UInt64 i );
Hash hashReal( Real real );
Hash hashPointer( const Void *p );
Hash mixHash( Hash xHash, Hash yHash );

// arraylist API --------------------------------------------------------------
Void  pushArrayList( ArrayList *array, Value val );
Value popArrayList( ArrayList *array );

// utility macros -------------------------------------------------------------
#define asArrayList(val) (asObjType(ArrayList, val))
#define asTable(val)     (asObjType(Table, val))

#endif
