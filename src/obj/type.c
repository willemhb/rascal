#include "obj/type.h"


type_t *val_type( value_t val )
{
  switch (val_tag(val))
    {
    case IMM_TAG: return ImmTypes[imm_tag(val)];
    case PTR_TAG: return PointerType;
    case FIX_TAG: return FixnumType;
    case OBJ_TAG: return obj_type( as_obj(val) );
    default:      return RealType;
    }
}

type_t *obj_type( object_t *obj )
{
  assert( obj );
  return (type_t*)hdr_data(obj->type);
}
