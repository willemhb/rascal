#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "rt.h"
#include "utils/arr.h"
#include "obj/obj.h"
#include "obj/repr.h"
#include "obj/heap.h"
#include "obj/stack.h"

// static helpers -------------------------------------------------------------
static void guard_gc( size_t n )
{
  if (Heap->allocated + n >= Heap->next_gc)
    collect_garbage();
}

// core memory api ------------------------------------------------------------
// object model core dispatch functions ---------------------------------------
val_t construct(init_t *ini)
{
  type_t  type = ini->type;

  if (ini->repr == NULL)
    ini->repr = Reprs[type];

  repr_t *repr = ini->repr;

  if (repr->val_tag == OBJECT || repr->val_tag == IMMEDIATE)
    {
      obj_t *out = mk_obj(ini);
      return as_val(out);
    }

  val_t buf = 0;

  if (repr->do_init_vals(&buf, ini->val))
    type_error( repr_name(repr),
		ini->val,
		repr->val_type );

  return buf|repr->val_tag;
}

obj_t *mk_obj(init_t *ini)
{
  obj_t *out = new(ini);

  if (!flag_p(ini->fl, INIT_NONE))
    init( out, ini );

  return out;
}

obj_t *new(init_t *init)
{
  obj_t *out; repr_t *repr = Reprs[init->type];
  
  if (init->obj != NULL && flag_p(init->fl, STATIC_OBJ))
      out = init->obj;

  else if (repr->do_new)
    out = repr->do_new(init);

  else
    out = alloc( repr->base_size );
  
  return out;
}

void init(obj_t *obj, init_t *init )
{
  repr_t *repr = Reprs[init->type];

  if (!flag_p(init->fl, INIT_SEQ))
    init_obj( obj, init );

  if (repr->do_init)
    repr->do_init( obj, init );
}

void finalize(obj_t **buffer)
{
  if (buffer == NULL)
    return;

  obj_t *obj  = (*buffer);
  obj_t *next = obj->next;

  obj->next = NULL;
  *buffer   = next;
  
  repr_t *repr = Reprs[obj->type];
  
  if (repr->do_finalize)
    repr->do_finalize(obj);

  dealloc(obj, repr->base_size);
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
  Heap->allocated -= n;
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
    Heap->allocated += diff;

  else
    Heap->allocated -= diff;

  return ptr;
}

void copy(void *dst, void *src, size_t n)
{
  memcpy( dst, src, n );
}


void *dup_bytes( void *ptr, size_t n )
{
  void *cpy = alloc( n );
  copy( cpy, ptr, n );
  return cpy;
}

obj_t *dup_obj( obj_t *ptr )
{
  return dup_bytes( ptr, sizeof_obj(ptr) );
}

// memory management ----------------------------------------------------------
void mark_val(val_t val)
{
  if (val_tag(val) == OBJECT)
    mark_obj(as_obj(val));
}

void trace_val(val_t val)
{
  trace_obj(as_obj(val));
}

void trace_obj(obj_t *obj)
{
  repr_t *repr = Reprs[obj->type];
  
  repr->do_trace(obj);
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

void collect_garbage( void )
{
  Heap->collecting = true;
  size_t allocated = Heap->allocated;

  gc_roots();
  gc_mark();
  gc_sweep();

  Heap->collecting = false;
  resize_heap( (obj_t*)Heap, allocated, Heap->allocated );
}

#include "obj/atom.h"
#include "obj/ns.h"
#include "obj/port.h"
#include "obj/symt.h"
#include "obj/vm.h"

static void gc_roots( void )
{
  Heap->collecting = true;

  atom_mark();
  ns_mark();
  obj_mark();
  port_mark();
  repr_mark();
  symt_mark();
  vm_mark();
}

static void gc_mark( void )
{
  while (((stack_t*)Heap->gray_stack)->len > 0)
    {
      val_t   val  = pop( Heap->gray_stack );
      type_t  type = typeof_val(val);
      repr_t *repr = Reprs[type];
      repr->do_trace(as_obj(val));
    }
}

static void gc_sweep( void )
{
  obj_t **buffer = &Heap->objects;

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
