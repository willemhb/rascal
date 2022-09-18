#include <string.h>

#include "vm.h"
#include "memory.h"
#include "table.h"
#include "value.h"

// forward declarations -------------------------------------------------------
static Void    resizeTable( Table *table, Arity newCap );
static Void    growTable( Table *table );
static Void    shrinkTable( Table *table );
static Void    rehash( Entry **oldData, Arity oldCap, Entry **newData, Arity newCap );
static Entry **tableLocate( Table *table, Value key, Hash *buf );
static Void    addEntry( Table *table, Entry **location, Value key, Hash hash );
static Entry  *popEntry( Table *table, Entry **location );

// hashing functions ----------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

Hash hashCstring( const Char* cstr )
{
  return hashMemory( (const UInt8*)cstr, strlen(cstr) );
}

Hash hashMemory( const UInt8 *bytes, Arity nBytes )
{
  Hash out = FNV64_OFFSET;
  for (Arity i=0; i<nBytes; i++)
    {
      out ^= bytes[i];
      out *= FNV64_PRIME;
    }

  return out;
}

Hash hashInt( uint64_t key )
{
    key = (~key) + (key << 21);            // key = (key << 21) - key - 1;
    key =   key  ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key =  key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key =  key ^ (key >> 28);
    key =  key + (key << 31);
    return key;
}

Hash hashReal( Real key )
{
  return hashInt( AS_VALUE(key, 0) );
}

Hash hashPointer( const Pointer p )
{
  return hashInt( AS_VALUE(p, 0) );
}

Hash mixHash( Hash xHash, Hash yHash )
{
  return hashInt( xHash ^ yHash );
}

// table implementations ------------------------------------------------------
#define TABLE_MIN_NKEYS       8
#define TABLE_RESIZE_PRESSURE 0.75
#define TABLE_RESIZE_FACTOR   2.0


Void initTable( Table *table )
{
  initObject( (Obj*)table, OBJ_TABLE );
  table->length   = 0;
  table->capacity = TABLE_MIN_NKEYS;
  table->data     = ALLOCATE( Tuple*, table->capacity );
}

Void freeTable( Table *table )
{
  Size oldSize    = table->capacity * sizeof(Tuple*);
  Size newSize    = TABLE_MIN_NKEYS * sizeof(Tuple*);

  table->length   = 0;
  table->capacity = TABLE_MIN_NKEYS;
  table->data     = (Tuple**)reallocate( table->data, NULL, oldSize, newSize );
}

static Entry *popEntry( Table *table, Entry **location )
{
  assert( location >= table->data
	  && location <= table->data + table->capacity );
  assert( *location != NULL );

  Entry *out = *location;
  *location  =  NULL;
  table->length--;
  shrinkTable( table );

  return out;
}

static Void   addEntry( Table *table, Entry **location, Value key, Hash hash)
{
  Entry *new = newTuple( 3 );

  new->space[0] = key;
  new->space[1] = NIL;
  new->space[2] = hash;

  *location = new;

  table->length++;
  growTable( table );
}

static Void rehash( Entry **oldData, Arity oldCap, Entry **newData, Arity newCap)
{
  for (Arity i=0; i<oldCap; i++)
    {
      if (oldData[i] == NULL)
	continue;

      Entry *entry = oldData[i];
      Hash   hash  = entry->space[2] & HASH_MASK;
      Arity  index = hash & (newCap-1);

      while (newData[index])
	{
	  index++;

	  if (index >= newCap)
	    index = 0;
	}

      newData[index] = entry;
    }
}

static Void resizeTable( Table *table, Arity newCap )
{
  Arity oldCap     = table->capacity;
  Entry **oldData  = table->data;
  Entry **newData  = ALLOCATE( Entry*, table->capacity );

  rehash( oldData, oldCap, newData, newCap );
  FREE(Entry*, oldData);

  table->capacity = newCap;
  table->data     = newData;
}

static Void growTable( Table *table )
{
  if ( table->length >= table->capacity * TABLE_RESIZE_PRESSURE )
    resizeTable( table, table->capacity * TABLE_RESIZE_FACTOR );
}

static Void shrinkTable( Table *table )
{
  if (table->capacity > TABLE_MIN_NKEYS
      && table->length < table->capacity / TABLE_RESIZE_FACTOR * TABLE_RESIZE_PRESSURE)
    resizeTable( table, table->capacity / TABLE_RESIZE_FACTOR );
}

static Entry **tableLocate( Table *table, Value key, Hash *buf )
{
  Hash  hash  = hashValue( key );

  if (buf)
    *buf = hash;
  
  Arity index = hash & (table->capacity-1);

  Entry *entry;

  while ((entry=table->data[index]))
    {
      if (hash == entry->space[2] && equalValues(key, entry->space[0]))
	break;

      index++;

      if (index >= table->capacity)
	index = 0;
    }

  return table->data + index;
}

Bool tableGet( Table *table, Value key, Value *buffer )
{
  Entry **location = tableLocate( table, key, NULL );

  if (*location)
    {
      if (buffer)
	*buffer = (*location)->space[1];

      return true;
    }

  if (buffer)
    *buffer = NIL;

  return false;
}

Bool tableSet( Table *table, Value key, Value val, Value *buffer )
{
  Entry **location = tableLocate( table, key, NULL );

  if (*location)
    {
      if (*buffer)
	*buffer = (*location)->space[1];

      (*location)->space[1] = val;
      return true;
    }

  return false;
}

Bool tablePut( Table *table, Value key, Entry **buffer )
{
  Hash h;

  Entry **location = tableLocate( table, key, &h );
  Bool out;

  if ((out=*location == NULL))
      addEntry( table, location, key, h );

  if (buffer)
    *buffer = *location;

  return out;
}

Entry *tablePop( Table *table, Value key )
{
  Entry **location = tableLocate( table, key, NULL );

  if (*location == NULL)
    return NULL;

  Entry *out = popEntry( table, location );
  return out;
}

// initialization -------------------------------------------------------------
Table SymbolTable;

Void TableInit( Void )
{
  vm.symbols  = &SymbolTable;
  vm.nSymbols = 0;

  initTable( &SymbolTable );
}
