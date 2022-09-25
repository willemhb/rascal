#include "val.h"
#include "obj/obj.h"
#include "obj/repr.h"
#include "obj/atom.h"

type_t typeof_val(val_t x)
{
  switch (val_tag(x))
    {
    case SMALL:     return small_type(x);
    case IMMEDIATE: return imm_type(x);
    case INTEGER:
    case ARITY:
    case POINTER:   return int_type;
    case OBJECT:    return as_obj(x)->type;
    default:        return real_type;
    }
}

repr_t *reprof_val(val_t x)
{
  return Reprs[typeof_val(x)];
}

int_t cmp_vals(val_t x, val_t y)
{
  if (x == y)
    return 0;
  
  repr_t *rx = reprof_val(x), *ry = reprof_val(y);

  if (rx->val_type != ry->val_type)
    return ord_uint(rx->val_type, ry->val_type);

  if (rx->cmp)
    return rx->cmp( x, y );

  return ord_uint( x, y );
}

hash_t hash_val( val_t x )
{
  repr_t *rx = reprof_val(x);

  if (rx->hash)
    return rx->hash(x);

  if (val_tag(x) == POINTER || val_tag(x) == OBJECT)
    return mix_hashes( rx->val_type, hash_ptr(as_ptr(x)));

  return hash_int(x);
}
