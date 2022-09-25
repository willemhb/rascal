#include "obj/obj.h"
#include "obj/repr.h"
#include "obj/stack.h"
#include "obj/heap.h"
#include "utils/arr.h"
#include "mem.h"


bool is_famous( obj_t *obj )
{
  return flag_p( obj->flags, FAMOUS_OBJ );
}

bool is_static( obj_t *obj )
{
  return flag_p( obj->flags, STATIC_OBJ );
}

bool has_static( obj_t *obj )
{
  return flag_p( obj->flags, STATIC_DATA );
}

// implementation and fallback methods for object types
void init_obj(obj_t *obj, type_t type, flags_t fl)
{
  obj->type  = type;
  obj->gray  = true;
  obj->black = false;
  obj->flags = fl&UINT16_MAX;

  if (flag_p(fl, STATIC_OBJ))
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

  if (Reprs[obj->type]->do_trace)
    push( Heap->gray_stack, as_val(obj) );
}

IMPL_FINALIZE(obj)
{
  if (Reprs[obj->type]->do_finalize)
    Reprs[obj->type]->do_finalize(obj);

  dealloc(obj, sizeof(obj));
}
