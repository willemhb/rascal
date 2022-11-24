#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "array.h"

/* local utilities & convenience */
/* gc guards */
void gc_check( size_t n_bytes )
{
  if ( Heap.bytes_allocated + n_bytes > Heap.next_cycle )
    collect_garbage();
}

void gc_array_check( size_t count, size_t ob_size )
{
  gc_check(count * ob_size);
}

void *heap_alloc( size_t n_bytes )
{
  Heap.bytes_allocated += n_bytes;
  void *out             = alloc_s(malloc, n_bytes);

  init_memory(out, n_bytes);

  return out;
}

void *heap_array_alloc( size_t count, size_t ob_size )
{
  Heap.bytes_allocated += count * ob_size;
  void *out             = alloc_s(calloc, count, ob_size);

  init_array_memory(out, count, ob_size);

  return out;
}

void heap_dealloc( void *ptr, size_t n_bytes )
{
  Heap.bytes_allocated -= n_bytes;
  free(ptr);
}

void heap_array_dealloc( void *ptr, size_t count, size_t ob_size )
{
  heap_dealloc(ptr, count*ob_size);
}

void *heap_resize( void *ptr, size_t n_old, size_t n_new )
{
  if ( n_old > n_new )
    {
      Heap.bytes_allocated -= n_old - n_new;

      return alloc_s(realloc, ptr, n_new);
    }

  else if ( n_old < n_new )
    {
      Heap.bytes_allocated += n_new - n_old;
      ptr                   = alloc_s(realloc, ptr, n_new);
      
      init_memory((uchar*)ptr+n_old, n_new-n_old);

      return ptr;
    }

  else
    return ptr;
}

void *heap_array_resize( void *ptr, size_t n_old, size_t n_new, size_t ob_size )
{
  return heap_resize(ptr, n_old*ob_size, n_new*ob_size);
}

void init_memory( void *spc, size_t n_bytes )
{
  memset(spc, 0, n_bytes);
}

void init_array_memory( void *spc, size_t count, size_t ob_size )
{
  memset(spc, 0, count*ob_size);
}

/* external API */
void *alloc( size_t n_bytes )
{
  gc_check(n_bytes);
  
  return heap_alloc(n_bytes);
}

void *array_alloc( size_t count, size_t ob_size )
{
  gc_array_check(count, ob_size);
  
  return heap_array_alloc(count, ob_size);
}

void *resize( void *ptr, size_t n_old, size_t n_new )
{
  if ( n_old < n_new )
    gc_check(n_new - n_old);
  
  return heap_resize(ptr, n_old, n_new);
}

void *array_resize( void *ptr, size_t n_old, size_t n_new, size_t ob_size )
{
  if ( n_old < n_new )
    gc_array_check(n_new - n_old, ob_size);

  return heap_array_resize(ptr, n_old, n_new, ob_size);
}

void dealloc( void *ptr, size_t n_bytes )
{
  heap_dealloc(ptr, n_bytes);
}

void array_dealloc( void *ptr, size_t count, size_t ob_size )
{
  heap_array_dealloc(ptr, count, ob_size);
}

/* garbage collector */
extern void mark_vm_roots( void );
extern void mark_io_roots( void );
extern void mark_obj_roots( void );

void gc_mark( void );
void gc_trace( void );
void gc_sweep( void );

void mark_value( value_t value );
void mark_values( value_t *values, size_t count );
void mark_objs( object_t **objects, size_t count );
void mark_obj( object_t *obj );
void trace_obj( object_t *obj );

/* mark and trace helpers */
void mark_value( value_t x )
{
  if ( is_obj(x) )
    mark_obj( as_obj(x) );
}

void mark_obj( object_t *obj )
{
  if ( obj == NULL )
    return;

  if ( obj->black )
    return;

  obj->black = true;

  if ( obj->gray )
    array_add(Heap.gray_objects, 1, tag_obj(obj));
}

void trace_obj( object_t *obj )
{
  if ( obj == NULL )
    return;

  mark_obj((object_t*)obj->type);
  mark_value(obj->_meta);

  obj->gray          = false;

  type_t      *type  = obj->type;
  fieldspec_t *specs = type->fields;
  size_t           i = 0, asize;

  value_t   value,   *values;
  object_t *object, **objects;

  while ( i < type->n_fields )
    {
      fieldspec_t spec = specs[i];

      switch ( spec.repr )
	{
	case repr_Cdata ... repr_Cstring:
	  break;

	case repr_value:
	  value = *(value_t*)(obj->space + spec.offset);
	  mark_value(value);
	  break;

	case repr_object:
	  object = *(object_t**)(obj->space + spec.offset);
	  mark_obj(object);
	  break;

	case repr_values:
	  values = *(value_t**)(obj->space + spec.offset);
	  asize  = *(size_t*)(obj->space + spec.offset + sizeof(value_t*));
	  mark_values(values, asize);
	  break;

	case repr_objects:
	  objects = *(object_t***)(obj->space + spec.offset);
	  asize   = *(size_t*)(obj->space + spec.offset + sizeof(object_t**));
	  mark_objs(objects, asize);
	  break;
	}
    }
}

/* main GC phases */
void gc_mark( void )
{
  mark_vm_roots();
  mark_io_roots();
  mark_obj_roots();
}

void gc_trace( void )
{

  while ( Heap.gray_objects->count > 0 )
    {
      value_t   next = array_pop(Heap.gray_objects, 1);
      object_t *obj  = as_obj(next);
      obj->gray      = false;
      trace_obj(obj);
    }
}

void gc_sweep( void )
{
  object_t **prev = &Heap.live_objects, *curr;

  while ((curr=*prev))
    {
      if (curr->black)
	{
	  // reset in preparation for next cycle
	  curr->black = false;
	  curr->gray  = true;

	  // update
	  prev = &curr->next;
	}

      else
	{
	  // remove curr from live objects list
	  *prev = curr->next;

	  // cleanup curr
	  free_obj(curr);
	}
    }
}

/* GC entry point */
void collect_garbage( void )
{
  /* mark all roots */
  gc_mark();
  
  /* process gray stack */
  gc_trace();

  /* free unused */
  gc_sweep();
}
