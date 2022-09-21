#include "obj/obj.h"
#include "mem.h"


void init_obj(obj_t *obj, val_type_t type, flags_t fl)
{
  obj->type  = type;
  obj->arity = 0;
  obj->gray  = true;

  if (is_flag( fl, MEM_STATIC))
    {
      obj->next  = NULL;
      obj->black = true;
    }

  else
    {
      obj->next = ObList;      
    }
}
