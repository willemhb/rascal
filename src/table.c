#include "table.h"
#include "number.h"
#include "memory.h"


const Double TableLoadFactor = .75;
const Size TableMinCap = 8;


// internal helpers
Size padTableLength( Size newCount, Size oldCap )
{
  Size newCap = oldCap;

  while ( newCount < newCap*TableLoadFactor/2 ) newCap >>= 1;
  while ( newCount > newCap*TableLoadFactor ) newCap <<= 1;

  newCap = max(newCap, TableMinCap);

  return newCap;
}

// implementations
// ReadTable implementation
ReadTable *createReadTable( Void )
{
  return (ReadTable*)create( &ReadTableType );
}

Void initReadTable( ReadTable *created )
{
  created->count    = 0;
  created->capacity = TableMinCap;
  created->entries  = allocArray(created->capacity, sizeof(Object**));
}

ReadTable *newReadTable( Void )
{
  ReadTable *created = createReadTable();
  initReadTable(created);

  return created;
}

Void rehashReadTable( ReadTable *table, Size oldCap, Object **oldEntries );

Void resizeReadTable( ReadTable *table, Size newCount )
{
  Size oldCap = table->capacity;
  table->count = newCount;
  table->capacity = padTableLength(newCount, table->capacity);

  if ( table->capacity != oldCap )
    {
      Object **oldEntries = table->entries;
      table->entries = allocArray(table->capacity, sizeof(Object**));
      rehashReadTable(table, oldCap, oldEntries);
      deallocArray(oldEntries, oldCap, sizeof(Object*));
    }
}

Void resetReadTable( ReadTable *table )
{
  resizeReadTable(table, 0);
  initArrayMemory(table->entries, table->capacity, sizeof(Object*));
}

Void addReadTableHandler( ReadTable *table, Char dispatch, Value handler )
{
  Value key = rlAscii(dispatch);
  ULong hash = hashULong(key);
  Size index = hash & (table->capacity-1);

  while ( table->entries[index] )
    {
      if ( asCons(table->entries[index])->car == key )
	{
	  asCons(table->entries[index])->cdr = handler;
	  return;
	}

      index = (index+1) & (table->capacity-1);
    }

  table->entries[index] = (Object*)newCons(key, handler);
  resizeReadTable(table, table->count+1);
}

Value getReadTableHandler( ReadTable *table, Char dispatch )
{
  Value key = tagValue(dispatch, ASCIITAG);
  ULong hash = hashULong(key);
  Size index = hash & (table->capacity-1);

  while ( table->entries[index] )
    {
      if ( asCons(table->entries[index])->car == key )
	  return asCons(table->entries[index])->cdr;

      index = (index+1) & (table->capacity-1);
    }

  return rlNul;
}

Void rehashReadTable( ReadTable *table, Size oldCap, Object **oldEntries )
{
  for ( Size i=0, moved=0; i < oldCap && moved < table->count; i++ )
    {
      if ( !oldEntries[i] )
	continue;

      Cons* entry = (Cons*)oldEntries[i];
      ULong hash = hashULong(entry->car);
      Size index = hash & (table->capacity-1);

      while ( table->entries[index] )
	index = (index+1) & (table->capacity-1);

      table->entries[index] = (Object*)entry;
      moved++;
    }
}

// globals
Type ReadTableType =
  {
    {
      .dtype = &TypeType.obj
    },

    "read-table",
    sizeof(ReadTable)
  };
