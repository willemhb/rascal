#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"

// convenience macros
#define TABLE					\
  OBJECT					\
  Size     count;				\
  Size     capacity;				\
  Object **entries;

// C types
typedef struct ReadTable
{
  TABLE
} ReadTable;

// forward declarations
ReadTable *createReadTable( Void );
Int        releaseReadTable( ReadTable *table );
Void       initReadTable( ReadTable *table, Size nKeys );
ReadTable *newReadTable( Size nKeys );

#define    asReadTable( x ) ((ReadTable*)asObject(x))
#define    isReadTable( x ) valueIsType(x, &ReadTableType)

Void       resizeReadTable( ReadTable *table, Size newCount );
Void       resetReadTable( ReadTable *table );
Void       addReadTableHandler( ReadTable *table, Char dispatch, Value handler );
Value      getReadTableHandler( ReadTable *table, Char dispatch );

// globals
extern Type ReadTableType;

#endif
