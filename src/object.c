#include <stddef.h>
#include <stdarg.h>

#include "object.h"
#include "memory.h"
#include "table.h"
#include "symbol.h"
#include "utils.h"

extern heap_t Heap;

void init_object( object_t *object, repr_t repr, bool isStatic )
{
  if (isStatic)
    {
      object->next    = tag_obj(NULL, repr);
      object->gcbits  = STATIC;
    }

  else
    {
      object->next = tag( Heap.objects, repr );
      Heap.objects = object;
    }
}

extern mark_fn_t Mark[N_REPR];

void mark_object( object_t *object )
{
  if (object == NULL)
    return;

  if (object->gcbits > WHITE)
    return;

  if (Mark[object->repr])
    {
      add_gray( &Heap, object );
      object->gcbits = GRAY;
    }

  else
    {
      object->gcbits = BLACK;
    }
}

void unmark_object( object_t *object )
{
  if (object == NULL)
    return;

  if (object->gcbits == STATIC)
    return;

  if (object->repr == SYMBOL && !sym_isGensym(object))
    return;

  object->gcbits = WHITE;
}

extern free_fn_t Free[N_REPR];

void free_object( object_t *object )
{
  if (object == NULL)
    return;

  if (Free[object->repr])
    Free[object->repr](object);

  else
    dealloc( object, obj_sizeof(object) );
}

extern size_fn_t Size[N_REPR];
extern size_t    BaseSize[N_REPR];

size_t obj_size( object_t *object )
{
  if (object == NULL)
    return 0;

  size_t total = BaseSize[object->repr];

  if (Size[object->repr])
    total += Size[object->repr](object);

  return total;
}

extern hash_fn_t Hash[N_REPR];

hash_t obj_hash( object_t *object )
{
  assert(object != NULL);

  if (Hash[object->repr])
    return Hash[object->repr](object);

  return hash_long(tag(object, object->repr));
}
