#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "list.h"
#include "number.h"
#include "map.h"


// forward declarations -------------------------------------------------------
static Arity mapNodeArity( Map *m );
static Bool  mapCanGrow( Map *m );
static Bool  mapHasIndex( Map *m, Int i );
static Int   mapFirstIndex( Map *m );
static Int   mapLastIndex( Map *m );
static Map  *updatePath( MapPathBuffer *m, Value val );
static Void  initMap( Map *m, Value *args, Arity nArgs );
static Map  *mapGrow( Map *m, Bool inPlace );
static Map  *mapShrink( Map *m, Bool inPlace );

// local helpers --------------------------------------------------------------
static inline Arity mapNodeArity( Map *m )
{
  return popcount( mapBitmap( m ) );
}

static inline Bool mapCanGrow( Map *m )
{
  return mapLength( m ) < HAMT_MAX_NODE_COUNT;
}

static inline Int mapFirstIndex( Map *h )
{
  if (h->bitmap)
    return countLz( h->bitmap );
  return -1;
}

static inline Int mapLastIndex( Map *h )
{
  if (h->bitmap)
    return 31 - countTz( h->bitmap );
  return -1;
}

static inline Bool  MapHasIndex( Map *h, Arity i )
{
  return i < HAMT_MAX_NODE_COUNT
    &&   !!((1<<i) & h->object.arity);
}

static inline Bool MapGetIndex( Map *h, Arity i )
{
  return popcount( ((1ul<<i)-1) & ((1ul<<countLz(h->bitmap))-1) );
}

static Map *updatePath( MapPathBuffer *buffer, Value val )
{
  Arity loc; Map *node, *pnode;
  
  for (Arity i=0; i < buffer->count; i++)
    {
      node               = (Map*)duplicateObject( (Obj*)buffer->nodes[i] );
      buffer->nodes[i]   = node;

      if (i > 0)
	getMapElements(pnode)[loc] = node;

      pnode              = node;
      loc                = buffer->indices[i];
    }

  assert( mapLeaf(node) );
  
  mapVal(node) = val;
  return buffer->nodes[0];
}

// implementations ------------------------------------------------------------
Map *newMap( Value *args, Arity nArgs )
{
  Map  *out = (Map*)allocateObject( VAL_MAP, nArgs );
  initMap( out, args, nArgs );

  return out;
}

void initMap( Map *h, Value *args, Arity nArgs )
{
    
}

static Map *mapGrow( Map *m )
{
  
  assert( mapCanGrow( m ) );
  
  Arity oldLength = mapLength( m );
  Arity newLength = oldLength * 2;
  
}


// map implementation ---------------------------------------------------------
Value *mapGet( Map *m, Value key, MapPathBuffer *buffer )
{
  Hash h = hashValue( key );

  for (Arity _=0; _ < MAP_MAX_DEPTH; _++)
    {
      UInt32 shift       = mapDepth(m) * 5;
      Arity  index       = (h & (0x1ful<<shift))>>shift;
      Arity  true_index  = MapGetIndex( ob, index );

      if (buffer)
	{
	  Arity count = buffer->count++;
	  buffer->nodes[count]   = ob;
	  buffer->indices[count] = true_index;
	}

      if (!MapHasIndex( ob, index ))
	return NULL;

      Value *entry = ob->data+true_index;
      
      if (IS_MAP(*entry))
	ob = AS_MAP(*entry);

      else if (IS_TUPLE(*entry))
	{
	  if (compareToEntry(key, h, as_entry(*entry)))
	    return entry;

	  return NULL;
	}

      List *collisions = as_list(*entry);

      while (collisions)
	{
	  Entry *entry0 = as_entry(collisions->head);

	  if (compareToEntry(key, h, entry0))
	    return entry;

	  collisions = collisions->tail;
	}
      return NULL;
    }
  unreachable();
}

Map *MapSet( Map *ob, Value key, Value val, Bool inPlace )
{
  MapPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };

  Value *location = mapRef( ob, key, &buffer );

  if (location == NULL)
    return NULL;

  else if (inPlace)
    entry_val(*location) = val;

  else
    ob = updatePath( &buffer, key );

  return ob;
}

Map *MapPut( Map *map, Value key, Bool inPlace )
{
  MapPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };
}
