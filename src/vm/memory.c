#include <stdlib.h>

#include "memory.h"
#include "utils.h"
#include "array.h"
#include "exec.h"
#include "symbol.h"

// root-marking includes

static const real_t heap_pressure = 0.75;
static const size_t heap_startcap = 65536 * 4 * sizeof(value_t);
static const size_t grays_mincap  = 512;

heap_t Heap;

vector_t GrayStack;

// heap implementation
void init_heap(heap_t *heap)
{
  heap->objects   = NULL;
  heap->alloccap  = heap_startcap;
  heap->allocated = 0;
  heap->grays     = &GrayStack;

  init_vector( heap->grays, true, array_shape_stack, grays_mincap );
}

void free_heap(heap_t *heap)
{

  for (object_t *obj=heap->objects;obj != NULL;)
    {
      object_t *tmp = obj;
      obj           = as_object(obj_next(tmp));
      free_object( obj );
    }

  free_vector((object_t*)heap->grays);
}

void mark_roots( void )
{
  symbol_roots();
  vm_roots();
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
