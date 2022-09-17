#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "list.h"
#include "number.h"
#include "array.h"
#include "hamt.h"

#define HAMT_MIN_NODE_COUNT  2
#define HAMT_MAX_NODE_COUNT 32
#define VECTOR_MAX_DEPTH     7
#define MAP_MAX_DEPTH       10
#define HASH_MASK           (~HEADER)

// C types --------------------------------------------------------------------
struct HAMTPathBuffer
{
  Arity count;
  HAMT *nodes[MAP_MAX_DEPTH];
  Arity indices[MAP_MAX_DEPTH];
};

#define as_entry(val)   (((Tuple*)AS_OBJ(val)))
#define entry_key(val)  (((Tuple*)AS_OBJ(val))->space[0])
#define entry_val(val)  (((Tuple*)AS_OBJ(val))->space[1])
#define entry_hash(val) (((Tuple*)AS_OBJ(val))->space[2])

// forward declarations -------------------------------------------------------
static Arity HAMTcount( HAMT *h );


// local helpers --------------------------------------------------------------
static inline Bool  HAMTisFull( HAMT *h )
{
  return h->length == HAMTcount( h );
}

static inline Bool  isMaxHAMTNodeSize( HAMT *h )
{
  return h->length == HAMT_MAX_NODE_COUNT;
}

static inline Arity HAMTcount( HAMT *h )
{
  return popcount( h->bitmap );
}

static inline Int HAMTFirstIndex( HAMT *h )
{
  if (h->bitmap)
    return countLz( h->bitmap );
  return -1;
}

static inline Int HAMTLastIndex( HAMT *h )
{
  if (h->bitmap)
    return 31 - countTz( h->bitmap );
  return -1;
}

static inline Bool  HAMTHasIndex( HAMT *h, Arity i )
{
  return i < 32
    &&   !!((1<<i) & h->object.arity);
}

static inline Bool HAMTGetIndex( HAMT *h, Arity i )
{
  return popcount( ((1<<i)-1) & ((1<<countLz(h->bitmap))-1) );
}

static inline Bool compareToEntry( Value key, Hash h, Entry *e )
{
  return (h&HASH_MASK) == (e->space[2]&HASH_MASK)
    && equalValues( key, e->space[0] );
}

static HAMT *updatePath( HAMTPathBuffer *buffer, Value val)
{  
  for (Arity i=buffer->count; i > 0; i--)
    {
      HAMT *node         = copyHAMT( buffer->nodes[i-1] );
      buffer->nodes[i-1] = node;
      Arity loc          = buffer->indices[i-1];
      
      if (i == buffer->count)
	{
	  if (node->object.type == OBJ_VECTOR)
	    node->data[loc] = val;
	  
	  else
	    {
	      Value entry = duplicateValue(node->data[loc],
					   &node->data[loc] );
	      entry_val(entry) = val;
	    }
	}

      else
	node->data[buffer->indices[i-1]] = AS_VALUE((Obj*)buffer->nodes[i], OBJECT);
    }
  return buffer->nodes[0];
}

// implementations ------------------------------------------------------------
HAMT *newHAMT( ObjType type )
{
  HAMT  *out    = ALLOCATE_OBJ( HAMT, type );

  initHAMT( out );

  return out;
}

HAMT *copyHAMT( HAMT *original )
{
  HAMT  *out   = DUP_OBJECT( HAMT, original );
  Value *space = DUP_ARRAY( Value, original->data, original->length, original->capacity );

  out->data    = space;
  out->cache   = NULL;

  return out;
}

void initHAMT( HAMT *h )
{
  h->length = HAMT_MIN_NODE_COUNT;
  h->height = 0;
  h->bitmap = 0;
  h->data   = ALLOCATE( Value, h->length );
  h->cache  = NULL;
}

void HAMTgrow( HAMT *h )
{
  
  assert(h->length < HAMT_MAX_NODE_COUNT);
  
  Arity oldLength = h->length;
  Arity newLength = oldLength * 2;

  h->data   = GROW_ARRAY( Value, &h->cache, h->data, oldLength, newLength );
  h->length = newLength;
}

// vector implementation ------------------------------------------------------
Value *vectorRef( Vector *ob, Arity index, HAMTPathBuffer *buffer )
{
  if (index > ob->object.arity)
    return NULL;

  for (Arity _=0; _ < VECTOR_MAX_DEPTH; _++)
    {
      UInt32 shift      = ob->height * 5;
      Arity local_index = (index & (0x1ful<<shift))>>shift;
      Arity true_index  = HAMTGetIndex( ob, local_index );
      
      if (buffer)
	{
	  Arity count = buffer->count++;
	  buffer->nodes[count]   = ob;
	  buffer->indices[count] = true_index;
	}

      if (ob->height == 0)
	return ob->data + true_index;

      ob = AS_VECTOR( ob->data[true_index] );
    }
  unreachable();
}

Vector *vectorSet( Vector *ob, Arity index, Value val, Bool inPlace )
{
  HAMTPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };

  Value *location = vectorRef( ob, index, &buffer );

  if (location == NULL)
    return NULL;

  else if (inPlace)
      *location = val;

  else
      ob = updatePath( &buffer, val );

  return ob;
}

// map implementation ---------------------------------------------------------
Value *mapRef( Map *ob, Value key, HAMTPathBuffer *buffer )
{
  Hash h = hashValue( key );

  for (Arity _=0; _ < MAP_MAX_DEPTH; _++)
    {
      UInt32 shift       = ob->height * 5;
      Arity  index       = (h & (0x1ful<<shift))>>shift;
      Arity  true_index  = HAMTGetIndex( ob, index );

      if (buffer)
	{
	  Arity count = buffer->count++;
	  buffer->nodes[count]   = ob;
	  buffer->indices[count] = true_index;
	}

      if (!HAMTHasIndex( ob, index ))
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
  HAMTPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };

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
  HAMTPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };
}
