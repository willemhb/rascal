#include "types.h"
#include "obj.h"

type_t typeof_obj(obj_t *obj)
{
  return obj->type;
}

type_t typeof_val(val_t val)
{
  switch ((val&TMASK))
    {
    case OBJ:    return as_obj(val)->type;
    case CHRTAG: return CHRTYPE;
    case NULTAG: return NULTYPE;
    default:     return REAL;
    }
}
