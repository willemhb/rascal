#include <string.h>

#include "table.h"
#include "number.h"
#include "memory.h"

const Double TableLoadFactor = .75;
const Size TableMinCap = 8;

// describe macros
#define MAKETABLE( T )				\
  T *make##T( Void )				\
  {						\
    return (T*)create(&T##Type);		\
  }

#define FREETABLE( T, E )					\
  Int free##T( T *table )					\
  {								\
    deallocArray(table->entries, table->capacity, sizeof(E*));	\
    return 0;							\
  }

#define INITTABLE( T, E )				\
  Void init##T( T *table, Size count )			\
  {							\
    table->count = 0;					\
    table->capacity = padTableLength(count, 0);		\
    table->entries = allocArray(count, sizeof(E*));	\
  }

#define RESIZETABLE( T, E )						\
  Void rehash##T( T *table, Size newCap );				\
  Size resize##T( T *table, Size newCount )				\
  {									\
    Size oldCap = table->capacity,					\
         newCap = padTableLength(newCount, oldCap);			\
    if ( oldCap != newCap )						\
      rehash##T(table, newCap);						\
    return (table->count=newCount);					\
  }

#define REHASHTABLE( T, E, getHash )					\
  Void rehash##T( T *table, Size newCap )				\
  {									\
    E** newEntries = allocArray(newCap, sizeof(E*));			\
    for ( Size i=0, n=0; i<table->capacity && n<table->count; i++ )	\
      {									\
	if ( table->entries[i] )					\
	  {								\
	    ULong hash = getHash(table->entries[i]);			\
	    Size  index = hash & (newCap-1);				\
	    while ( newEntries[index] )					\
	      index = (index+1) & (newCap-1);				\
	    newEntries[index] = table->entries[i];			\
	    n++;							\
	  }								\
      }									\
    deallocArray(table->entries, table->capacity, sizeof(E));		\
    table->entries = newEntries;					\
    table->capacity = newCap;						\
  }

#define ADDTOTABLE( T, E, K, hashKey, getKey, compare )		\
  E* addTo##T( T *table, K key )				\
  {								\
    ULong hash = hashKey(key);					\
    Size index = hash & (table->capacity-1);			\
    while ( table->entries[index] )				\
      {								\
	if ( compare(key, getKey(table->entries[index])) )	\
	  return table->entries[index];				\
	index = (index+1) & (table->capacity-1);		\
      }								\
    E *entry = table->entries[index] = make##E();		\
    init##E(entry, key, hash);					\
    resize##T(table, table->count+1);				\
    return entry;						\
  }

#define LOOKUPINTABLE( T, E, K, hashKey, getKey, compare )		\
  E* lookupIn##T( T *table, K key )					\
  {									\
    ULong hash = hashKey(key);						\
    Size index = hash & (table->capacity-1);				\
    while ( table->entries[index] )					\
      {									\
	if ( compare(key, getKey(table->entries[index])) )		\
	  return table->entries[index];					\
	index = (index+1) & (table->capacity-1);			\
      }									\
    return NULL;							\
  }

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
// symbol table implementation
static inline ULong getSymbolHash( Symbol *symbol ) { return symbol->hash; }
static inline Char *getSymbolKey( Symbol *symbol ) { return symbol->name; }
static inline Bool symbolCompare( Char *name, Char *symName )
{
  return strcmp(name, symName) == 0;
}

MAKETABLE(SymbolTable);
FREETABLE(SymbolTable, Symbol);
INITTABLE(SymbolTable, Symbol);
RESIZETABLE(SymbolTable, Symbol);
REHASHTABLE(SymbolTable, Symbol, getSymbolHash);
ADDTOTABLE(SymbolTable, Symbol, Char*, hashString, getSymbolKey, symbolCompare);
LOOKUPINTABLE(SymbolTable, Symbol, Char*, hashString, getSymbolKey, symbolCompare);

Symbol *internSymbol( Char *name )
{
  return addToSymbolTable(&TheSymbolTable, name);
}

// initialization
#define RASCAL_INIT_SYMBOL_COUNT 256

Void rlTableInit( Void )
{
  initSymbolTable(&TheSymbolTable, RASCAL_INIT_SYMBOL_COUNT);
}

// globals
SymbolTable TheSymbolTable =
  {
    {
      .dtype=&SymbolTableType.obj
    },

    .entries=NULL
  };

Type SymbolTableType =
  {
    {
      .dtype=&TypeType.obj
    },

    "symbol-table",
    sizeof(SymbolTable)
  };
