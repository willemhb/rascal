#include "obj.h"
#include "template/arr.h"
#include "utils/arr.h"
#include "mem.h"

// implementation and fallback methods for object types
void init_obj(obj_t *obj, type_t type, flags_t fl)
{
  obj->type  = type;
  obj->gray  = true;
  obj->black = false;
  obj->flags = fl&UINT16_MAX;

  if (flag_p(fl, mem_fl_static))
    obj->next = NULL;

  else
    {
      obj->next     = Heap.objects;
      Heap.objects = obj;
    }
}

// objs_t implementation
static const size_t objs_min_cap = 8;

PAD_STACK_SIZE(objs, obj_t*)
NEW_ARRAY(objs, obj_t*)
INIT_ARRAY(objs, obj_t*)
RESIZE_ARRAY(objs, obj_t*)
FINALIZE_ARRAY(objs, obj_t*)
MARK_ARRAY(objs)
