#include "value.h"
#include "object.h"
#include "utils.h"

value_t tag_real( real_t real, repr_t repr )
{
  (void)repr;

  return ((val_data_t)real).as_val;
}

value_t tag_int( int_t i, repr_t repr )
{
  (void)repr;
  return (((value_t)i)&~HTMASK)|INT_TAG;
}

value_t tag_imm( uint32_t i, repr_t r )
{
  return (((value_t)i)&~ITMASK)|((value_t)r<<32);
}

value_t tag_obj( object_t *obj, repr_t r )
{
  return ((value_t)obj)|OBJ_TAG|r;
}

value_t tag_ptr( void *ptr, repr_t r )
{
  if (r <= RECORD)
    return tag_obj(ptr, r);

  return ((value_t)ptr)|PTR_TAG;
}

void *ptr_as_ptr(ptr_t ptr)
{
  return ptr;
}

void *val_as_ptr(value_t val)
{
  if ((val&HTMASK)==OBJ_TAG)
    return (void*)(val&OBPTRMASK);

  return (void*)(val&PTRMASK);
}

repr_t obj_reprof( object_t *obj )
{
  assert(obj != NULL);

  return obj->repr;
}

repr_t val_reprof( value_t val )
{
  switch (val&(OBJ_TAG|SIGN))
    {
    case IMM_TAG: return val>>32&255;
    case INT_TAG: return INT;
    case PTR_TAG: return PTR;
    case OBJ_TAG: return val&15;
    default:      return REAL;
    }
}

extern size_t     BaseSizes[N_REPR];

size_t val_size( value_t val )
{
  if (is_obj(val))
    return obj_size(as_object(val));

  return BaseSizes[val_reprof(val)];
}

size_t val_hash( value_t val )
{
  if (is_obj(val))
    return obj_hash(as_object(val));

  return hash_long(val);
}

extern order_fn_t Order[N_REPR];

ord_t val_order( value_t x, value_t y )
{
  if (x == y)
    return 0;

  repr_t xr = val_reprof(x), yr = val_reprof(y);

  if (xr != yr)
    return cmp(xr, yr);

  if (xr <= RECORD)
    return Order[xr](as_object(x), as_object(y));

  switch (xr)
    {
    case BOOL: return cmp(as_bool(x), as_bool(y));
    case CHAR: return cmp(as_chr(x), as_chr(y));
    case INT:  return cmp(as_int(x), as_int(y));
    case PTR:  return cmp(as_ptr(x), as_ptr(y));
    case REAL: return cmp(as_real(x), as_real(y));
    default:   unreachable();
    }
}
