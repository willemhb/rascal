#include <stdlib.h>
#include <string.h>

#include "obj/type.h"

#include "vm/object.h"
#include "vm/memory.h"

#include "util/memory.h"

/* commentary */

/* C types */

/* globals */
static const double heap_load_factor = 0.75; // grow n_alloc if this much or more of the heap is used after a collection
static const size_t heap_grow_factor = 2;    // amount to grow the heap by
/* static helpers */
static void check_gc( size_t n_bytes )
 {
  if ( !Heap.collecting && Heap.n_alloc + n_bytes >= Heap.n_collect )
    collect_garbage();

  Heap.n_alloc += n_bytes;
}

static void array_check_gc( size_t count, size_t ob_size )
{
  check_gc(count * ob_size);
}

static void table_check_gc( size_t base, size_t count, size_t ob_size )
{
  check_gc(base + count * ob_size);
}

/* API */
/* alloc methods */
void *alloc( size_t n_bytes )
{
  check_gc(n_bytes);
  return malloc_s(n_bytes);
}

void *alloc_array( size_t count, size_t ob_size )
{
  array_check_gc(count, ob_size);
  return calloc_s(count, ob_size);
}

void *alloc_table( size_t base, size_t count, size_t ob_size )
{
  table_check_gc(base, count, ob_size);
  return oballoc_s(base, count, ob_size);
}

/* dealloc methods */
void  dealloc( void *ptr, size_t n_bytes )
{
  Heap.n_alloc -= n_bytes;
  free(ptr);
}

void dealloc_array( void *ptr, size_t count, size_t ob_size )
{
  dealloc(ptr, count * ob_size);
}

void dealloc_table( void *ptr, size_t base, size_t count, size_t ob_size )
{
  dealloc(ptr, base + count * ob_size);
}

/* adjust (resize) methods */
void *adjust( void *ptr, size_t old_size, size_t new_size )
{
  if ( old_size < new_size )
    {
      size_t diff = new_size - old_size;
      check_gc(diff);
    }

  else
    {
      size_t diff = old_size - new_size;
      Heap.n_alloc += diff;
    }

  void *out = realloc_s(ptr, new_size);

  return out;
}

void *adjust_array( void *ptr, size_t old_count, size_t new_count, size_t ob_size )
{
  return adjust(ptr, old_count * ob_size, new_count * ob_size);
}

void *adjust_table( void *ptr, size_t base, size_t old_count, size_t new_count, size_t ob_size )
{
  return adjust(ptr, base + old_count * ob_size, base + new_count * ob_size);
}

/* gc implementation */
/* phases */
void rl_gc_init( void );
void rl_gc_mark( void );
void rl_gc_trace( void );
void rl_gc_sweep( void );
void rl_gc_finalize( void );

/* toplevel */
void collect_garbage( void )
{
  rl_gc_init();
  rl_gc_mark();
  rl_gc_trace();
  rl_gc_sweep();
  rl_gc_finalize();
}

void rl_gc_init( void )
{
  Heap.collecting = true;
}

/* external marking functions */
extern void rl_vm_mark( void );
extern void rl_rl_mark( void );
extern void rl_obj_mark( void );

void rl_gc_mark( void )
{
  rl_vm_mark();
  rl_rl_mark();
  rl_obj_mark();
}

void rl_gc_trace( void )
{
  /* Until the gray stack is empty:
     
     1. pop an object from the gray stack.
     2. trace it! */

  while ( Heap.grays->len > 0 )
    {
      object_t   *object  = objects_pop(Heap.grays);
      datatype_t *objtype = object->type;

      objtype->methods->trace(object);

      object->gray = false;
    }
}

void rl_gc_sweep( void )
{
  object_t **spc = &Heap.live;

  while ( *spc )
    {
      object_t *obj = *spc;

      if (obj->black)
	{
	  obj->black = false;
	  obj->gray  = true;
	  spc        = &obj->next;
	}

      else
	{
	  *spc = obj->next;

	  free_object(obj);
	}
    }
}

void rl_gc_finalize( void )
{
  Heap.collecting = false;

  if ( Heap.n_alloc >= Heap.n_collect * heap_load_factor )
    Heap.n_collect *= heap_grow_factor;
}

/* runtime */
void rl_vm_memory_init( void ) {}
void rl_vm_memory_mark( void ) {}

/* convenience */
