#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "obj.h"
#include "rt.h"
#include "utils/arr.h"
#include "obj/cons.h"
#include "obj/atom.h"

// static helpers -------------------------------------------------------------
static void guard_gc( size_t n )
{
  if (Heap.allocated + n >= Heap.next_gc)
    collect_garbage();
}

// core allocation functions --------------------------------------------------
void *alloc(size_t n)
{
  assert( n > 0 );
  guard_gc( n );
  void* out = safe_alloc( malloc, n );
  return out;
}

void dealloc(void *ptr, size_t n)
{
  assert( n > 0 );
  Heap.allocated -= n;
  free(ptr);
}

void *resize_bytes( void *ptr, size_t old_n, size_t new_n )
{
  bool   grew = new_n > old_n;
  size_t diff = grew ? new_n - old_n : old_n - new_n;
  
  if (grew)
    guard_gc( diff );

  ptr = safe_alloc( realloc, ptr, new_n );

  if (grew)
    Heap.allocated += diff;

  else
    Heap.allocated -= diff;

  return ptr;
}

void copy(void *dst, void *src, size_t n)
{
  memcpy( dst, src, n );
}


// memory management ----------------------------------------------------------
void mark_val(val_t val)
{
  if (val_tag(val) == OBJECT)
    mark_obj(as_obj(val));
}

void mark_obj(obj_t *obj)
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  obj->black = true;

  if (ObjApis[obj->type].do_mark)
    {
      push( Heap.gray_stack, obj );
    }

  else
    obj->gray = false;
}

void trace_val(val_t val)
{
  trace_obj(as_obj(val));
}

void trace_vals(val_t *vals, arity_t n)
{
  for (arity_t i=0; i<n; i++)
    mark(vals[i]);
}

void trace_objs(obj_t **objs, arity_t n)
{
  for (arity_t i=0; i<n; i++)
    mark(objs[i]);
}

void trace_noop(void *spc, arity_t n)
{
  (void)spc;
  (void)n;
}

// gc toplevel
static void gc_roots( void );
static void gc_mark( void );
static void gc_sweep( void );
static const float gc_load_factor = 0.75;

void collect_garbage( void )
{
  Heap.collecting = true;

  gc_roots();
  gc_mark();
  gc_sweep();

  Heap.collecting = false;

  if (Heap.allocated >= Heap.next_gc * gc_load_factor)
    Heap.next_gc *= 2;
}

#include "exec.h"

static void gc_roots( void )
{
  exec_mark();
  atom_mark();
}

static void gc_mark( void )
{
  while ((Heap.gray_stack)->len > 0)
    {
      obj_t  *obj  = pop( Heap.gray_stack );
      type_t type  = obj->type;
      ObjApis[type].do_mark(obj);
      obj->gray    = true;
    }
}

static void gc_sweep( void )
{
  obj_t **buffer = &Heap.objects;

  while (*buffer != NULL)
    {
      if ((*buffer)->black)
	{
	  (*buffer)->black = false;
	  (*buffer)->gray  = true;
	  buffer           = &((*buffer)->next);
	}

      else
	finalize( buffer );
    }
}

// initialization
#include "obj/port.h"
#include "obj/clo.h"
#include "obj/envt.h"

void mem_init( void )
{
  
  static const size_t obj_base_sizes[num_val_types] =
    {
      [none_type] = 0, [any_type] = sizeof(val_t),

      [int_type]   = sizeof(int_t),
      [real_type]  = sizeof(real_t),
      [bool_type]  = sizeof(bool_t),
      [char_type]  = sizeof(char_t),

      [atom_type]  = sizeof(atom_t),
      [nil_type]   = sizeof(val_t),
      [cons_type]  = sizeof(cons_t),
      [func_type]  = sizeof(func_t),
      [port_type]  = sizeof(port_t),
      [str_type]   = sizeof(str_t),
      [table_type] = sizeof(table_t),
      [clo_type]   = sizeof(clo_t),
      [envt_type]  = sizeof(envt_t)
    };

  for (type_t t=1; t <  num_val_types; t++)
    {
      ObjApis[t] = (obj_api_t)
	{
	  .base_obj_size=obj_base_sizes[t],
	  .do_finalize=NULL,
	  .do_mark=NULL
	};
    }
}
