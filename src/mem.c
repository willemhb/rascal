#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "obj/obj.h"
#include "obj/repr.h"
#include "obj/heap.h"

// static helpers -------------------------------------------------------------
static void guard_gc( size_t n )
{
  if (Heap->allocated + n >= Heap->next_gc)
    collect_garbage();
}

// core memory api ------------------------------------------------------------
// object model core dispatch functions ---------------------------------------
obj_t *new(init_t *ini)
{
  obj_t *out;
  
  if (Reprs[ini->type]->new)
    out = Reprs[ini->type]->new( ini );

  else
    out = alloc( Reprs[ini->type]->base_size );

  init( out, ini );
  return out;
}

void init(obj_t *obj, init_t *init)
{
  init_obj(obj, init);

  if (Reprs[obj->type]->init)
    Reprs[obj->type]->init(obj, init);
}

void finalize(obj_t *obj, obj_t **prev)
{
  *prev = obj->next;
  obj->next = NULL;

  finalize_obj(obj);
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
