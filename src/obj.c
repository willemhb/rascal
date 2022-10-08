#include "obj.h"
#include "memory.h"
#include "array.h"

// object
void init_obj( object_t *obj, type_t type, flags_t flags )
{
  obj->next    = Heap.objects;
  Heap.objects = obj;

  obj->type    = type;
  obj->gray    = true;
  obj->black   = false;
  obj->flags   = flags;
}

void free_obj(object_t *obj)
{
  if (obj == NULL)
    return;

  type_t type = obj->type;

  if (Free[type])
    Free[type](obj);

  dealloc( obj, BaseSizes[type] );
}


void mark_objs(object_t **objs, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_obj(objs[i]);
}

void mark_obj(object_t *obj)
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  type_t t = rtypeof(obj);

  if (Mark[t])
    stack_push( Heap.grays, tag_ptr(obj, OBJ) );

  else
    obj->gray = false;
}
