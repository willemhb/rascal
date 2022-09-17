
#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

// C types ---------------------------------------------------------------------
typedef Tuple Entry;

struct Table
{
  OBJ_HEAD;
  DYNAMIC_ARRAY_SPEC(Tuple*);
};

// forward declarations -------------------------------------------------------
// hashing utilities ----------------------------------------------------------
Hash hashCstring( const Cstring cstr );
Hash hashMemory( const UInt8 *bytes, Arity nBytes );
Hash hashInt( UInt64 i );
Hash hashReal( Real real );
Hash hashPointer( const Pointer p );
Hash mixHash( Hash xHash, Hash yHash );

// user table type APIS -------------------------------------------------------

#endif
