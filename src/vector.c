#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "list.h"
#include "number.h"
#include "vector.h"

// forward declarations -------------------------------------------------------
static Arity Vectorcount( Vector *h );


// local helpers --------------------------------------------------------------
static inline Bool  MapNodeIsFull( Vector *h )
{
  return h->length == Vectorcount( h );
}

static inline Bool  isMaxVectorNodeSize( Vector *h )
{
  return h->length == Vector_MAX_NODE_COUNT;
}

static inline Arity Vectorcount( Vector *h )
{
  return popcount( h->bitmap );
}

static inline Int VectorFirstIndex( Vector *h )
{
  if (h->bitmap)
    return countLz( h->bitmap );
  return -1;
}

static inline Int VectorLastIndex( Vector *h )
{
  if (h->bitmap)
    return 31 - countTz( h->bitmap );
  return -1;
}

static inline Bool  VectorHasIndex( Vector *h, Arity i )
{
  return i < 32
    &&   !!((1<<i) & h->object.arity);
}

static inline Bool VectorGetIndex( Vector *h, Arity i )
{
  return popcount( ((1<<i)-1) & ((1<<countLz(h->bitmap))-1) );
}

static inline Bool compareToEntry( Value key, Hash h, Entry *e )
{
  return (h&HASH_MASK) == (e->space[2]&HASH_MASK)
    && equalValues( key, e->space[0] );
}

static Vector *updatePath( VectorPathBuffer *buffer, Value val)
{  
  for (Arity i=buffer->count; i > 0; i--)
    {
      Vector *node         = copyVector( buffer->nodes[i-1] );
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
Vector *newVector( ObjType type )
{
  Vector  *out    = ALLOCATE_OBJ( Vector, type );

  initVector( out );

  return out;
}

Vector *copyVector( Vector *original )
{
  Vector  *out   = DUP_OBJECT( Vector, original );
  Value *space = DUP_ARRAY( Value, original->data, original->length, original->capacity );

  out->data    = space;
  out->cache   = NULL;

  return out;
}

void initVector( Vector *h )
{
  h->length = Vector_MIN_NODE_COUNT;
  h->height = 0;
  h->bitmap = 0;
  h->data   = ALLOCATE( Value, h->length );
  h->cache  = NULL;
}

void Vectorgrow( Vector *h )
{
  
  assert(h->length < Vector_MAX_NODE_COUNT);
  
  Arity oldLength = h->length;
  Arity newLength = oldLength * 2;

  h->data   = GROW_ARRAY( Value, &h->cache, h->data, oldLength, newLength );
  h->length = newLength;
}

// vector implementation ------------------------------------------------------
Value *vectorRef( Vector *ob, Arity index, VectorPathBuffer *buffer )
{
  if (index > ob->object.arity)
    return NULL;

  for (Arity _=0; _ < VECTOR_MAX_DEPTH; _++)
    {
      UInt32 shift      = ob->height * 5;
      Arity local_index = (index & (0x1ful<<shift))>>shift;
      Arity true_index  = VectorGetIndex( ob, local_index );
      
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
  VectorPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };

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
Value *mapRef( Map *ob, Value key, VectorPathBuffer *buffer )
{
  Hash h = hashValue( key );

  for (Arity _=0; _ < MAP_MAX_DEPTH; _++)
    {
      UInt32 shift       = ob->height * 5;
      Arity  index       = (h & (0x1ful<<shift))>>shift;
      Arity  true_index  = VectorGetIndex( ob, index );

      if (buffer)
	{
	  Arity count = buffer->count++;
	  buffer->nodes[count]   = ob;
	  buffer->indices[count] = true_index;
	}

      if (!VectorHasIndex( ob, index ))
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
  VectorPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };

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
  VectorPathBuffer buffer = { .count = 0, .indices = { }, .nodes = { } };
}
