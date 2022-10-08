#include "types.h"
#include "obj.h"

type_t typeof_obj(object_t *obj)
{
  return obj->type;
}

type_t typeof_val(value_t value)
{
  switch ((val&TMASK))
    {
    case OBJ:     return as_obj(val)->type;
    case CHRTAG:  return CHRTYPE;
    case NULTAG:  return NULTYPE;
    case BOOLTAG: return BOOLTYPE;
    case INTTAG:  return INTTYPE;
    case PTRTAG:  return PTRTYPE;
    default:      return REAL;
    }
}
