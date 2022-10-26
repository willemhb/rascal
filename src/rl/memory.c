#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm/memory.h"
#include "obj/memory.h"

// globals
heap_t Heap;
objects_t Grays;
behaviors_t Inits, Marks, Unmarks, Finalizers;

// memory management
static const real_t heap_pressure = 0.75;

void *alloc( size_t n )
{
  if (Heap.allocated + n > Heap.alloccap)
    collect_garbage( );

  Heap.allocated += n;

  void *out = malloc_s( n );
  memset( out, 0, n );
  return out;
}

object_t *allocob( size_t n )
{
  heap_object_t *object = alloc( n + sizeof(heap_object_t*) );

  object->obj.allocated = true;
  object->next          = Heap.live_objects;
  Heap.live_objects     = object;

  return &object->obj;
}

void *resize( void *p, size_t o, size_t n )
{
  if (o < n)
    { 
      if (Heap.allocated + (n-o) > Heap.alloccap)
	collect_garbage( );

      Heap.allocated += (n-o);
    }

  else
    Heap.allocated -= (o-n);

  return realloc_s( p, n );
}

void dealloc( void *p, size_t n )
{
  Heap.allocated -= n;
  free( p );
}

void *duplicate_bytes( void *p, size_t n )
{
  void *pc = alloc( n );
  memcpy( pc, p, n );
  return pc;
}

char *duplicate_string( char *chars )
{
  assert( chars != NULL );
  return duplicate_bytes( chars, strlen(chars)+1 );
}

// gc entry point
void collect_garbage( void )
{
  // mark phase
  for (size_t i=0; i<Marks.length; i++)
    Marks.data[i]();

  while (Grays.length > 0)
    {
      object_t *obj   =objects_pop( &Grays.obj );
      type_t   *type  =obtype(obj);

      obj->gray = false;
      type->dtype->trace( obj );
    }

  // sweep phase
  heap_object_t **buf = &Heap.live_objects;

  while (*buf != NULL)
    {
      heap_object_t *obj = *buf;

      if (obj->obj.permanent)
	buf = &obj->next;

      else if (obj->obj.black)
	{
	  obj->obj.black = false;
	  buf            = &obj->next;
	}

      else
	{
	  *buf = obj->next;
	  obj_free( &obj->obj );
	}
    }

  // unmark phase
  for (size_t i=0; i<Unmarks.length; i++)
    Unmarks.data[i]();

  // adjust alloc cap if necessary
  if (Heap.allocated > Heap.alloccap * heap_pressure)
    Heap.alloccap *= 2;
}
