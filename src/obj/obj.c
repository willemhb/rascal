#include "obj/obj.h"
#include "obj/repr.h"
#include "obj/stack.h"
#include "obj/heap.h"
#include "mem.h"


bool is_famous( obj_t *obj )
{
  return is_flag( obj->flags, FAMOUS_OBJ );
}

bool is_static( obj_t *obj )
{
  return is_flag( obj->flags, STATIC_OBJ );
}

bool has_static( obj_t *obj )
{
  return is_flag( obj->flags, STATIC_DATA );
}

// implementation and fallback methods for object types
IMPL_INIT(obj)
{
  obj->type  = args->type;
  obj->gray  = true;
  obj->black = false;
  obj->flags = args->fl&UINT16_MAX;

  if (is_flag(args->fl, STATIC_OBJ))
    obj->next = NULL;

  else
    {
      obj->next     = Heap->objects;
      Heap->objects = obj;
    }
}

IMPL_MARK(obj)
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  obj->black = true;

  if (Reprs[obj->type]->trace)
    stack_push( Heap->gray_stack, as_val(obj) );
}

IMPL_FINALIZE(obj)
{
  if (Reprs[obj->type]->finalize)
    Reprs[obj->type]->finalize(obj);

  dealloc(obj, sizeof(obj));
}
