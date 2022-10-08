#include <stdlib.h>

#include "memory.h"
#include "array.h"
#include "utils.h"

// root-marking includes
#include "atom.h"
#include "read.h"
#include "port.h"
#include "envt.h"

static const size_t heap_inic     = 8192*sizeof(alist_t);
static const real_t heap_pressure = 0.75;

// heap implementation
void init_heap(heap_t *heap)
{
  // initialize heap by hand, since it bypasses the usual protocol
  heap->obj.next  = NULL;
  heap->obj.type  = HEAP;
  heap->obj.gray  = false;
  heap->obj.black = true;
  heap->obj.flags = 0;

  heap->allocated = 0;
  heap->alloccap  = heap_inic;

  heap->objects   = NULL;
  stack_t *gray   = malloc_s( sizeof(alist_t) );
  heap->grays     = gray;

  // do the same for the initial part of heap->grays
  gray->obj.next  = NULL;
  gray->obj.type  = ALIST;
  gray->obj.gray  = false;
  gray->obj.black = true;
  gray->obj.flags = Ctype_pointer;

  gray->len       = 0;
  gray->cap       = MinCs[STACK];
  gray->data      = alloc_vec(  gray->cap, value_t );

  // saved can be initialized the easy way
  heap->saved     = new_stack();
  init_stack(heap->saved);
}

void free_heap(object_t *obj)
{
  heap_t *heap = (heap_t*)obj;

  for (object_t *obj=heap->objects;obj != NULL;)
    {
      object_t *tmp = obj;
      obj        = tmp->next;
      free_obj( obj );
    }
  
  free_stack((object_t*)heap->grays);
  free( heap->grays );
}


void mark_roots( void )
{
  mark_obj((object_t*)Heap.saved);
  mark_obj((object_t*)&Symbols);
  mark_obj((object_t*)&Reader);
  mark_obj((object_t*)&Ins);
  mark_obj((object_t*)&Outs);
  mark_obj((object_t*)&Errs);
}

void collect_garbage(void)
{
  mark_roots();

  value_t buf;
  while (stack_pop(Heap.grays, &buf))
    {
      object_t *obj       = as_obj(buf);
      type_t type      = obj->type;
      mark_fn_t mark   = Mark[type];
      mark(obj);
      obj->gray        = false;
    }

  object_t **prev = &Heap.objects, *curr = Heap.objects;
  
  while (curr != NULL)
    {
      if (curr->black)
	{
	  curr->black = false;
	  curr->gray  = true;
	  prev        = &curr->next;
	  curr        = curr->next;
	}

      else
	{
	  object_t *obj  = curr;
	  *prev       = curr->next;
	  curr        = curr->next;

	  free_obj(obj);
	}
    }

  if (Heap.allocated * heap_pressure > Heap.alloccap)
    Heap.alloccap *= 2;
}

void *alloc(size_t n)
{
  if (Heap.allocated + n > Heap.alloccap)
    collect_garbage();
  
  void *out = malloc_s(n);
  memset(out, 0, n);
  return out;
}

void *duplicate_bytes(void *p, size_t n)
{
  void *dup = alloc(n);
  memcpy(dup, p, n);
  return dup;
}

char *duplicate_string(char *chars)
{
  return duplicate_bytes(chars, strlen(chars));
}

void* resize(void *p, size_t o, size_t n)
{
  if (o > n)
    Heap.allocated -= n;

  else
    {
      size_t diff = n - o;

      if (Heap.allocated + diff > Heap.alloccap)
	collect_garbage();

      Heap.allocated += diff;
    }

  return realloc_s(p, n);
}

void dealloc(void *p, size_t n)
{
  Heap.allocated -= n;
  free(p);
}

void memory_init( void )
{
  init_heap(&Heap);
}
