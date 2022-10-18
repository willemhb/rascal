#include "rl/value.h"
#include "obj/type.h"
#include "obj/boxed.h"
#include "obj/cvalue.h"

repr_t val_repr( value_t val )
{
  switch(val&VT_MASK)
    {
    case IMM_TAG:  return imm_repr(val);
    case PTR_TAG:  return ptr_repr(val);
    case FIX_TAG:  return fix_repr(val);
    case OBJ_TAG:  return ob_repr(val);
    case HDR_TAG:  return hdr_repr(val);
    default:       return real_repr(val);
    }
}

type_t *val_type( value_t val )
{
  repr_t r = val_repr( val );

  return r < SYMBOL ? as_box(val)->type : ReprTypes[r];
}

size_t val_size( value_t val )
{
  if (is_obj(val))
    return obj_size( as_obj(val) );

  return repr_size( val_repr(val) );
}

rl_value_t val_unwrap( value_t val )
{
  switch ( val_repr(val) )
    {
    case CVALUE:
      return cv_unwrap( as_cvalue(val) );

    case RECORD ... AMT:
    case TYPE ... UPVALUE:
    case FIXNUM:
    case POINTER:
      return (rl_value_t){ val&PTR_MASK };

    case NUL:
      return (rl_value_t){ 0ul };

    case BOOLEAN ... REAL32:
      return (rl_value_t){ val&IMM_MASK };

    case REAL:
      return (rl_value_t){ val };
    }
  unreachable();
}

size_t val_unbox( value_t val, size_t size, void *spc )
{
  if (size == 0)
    return 0;

  switch ( val_repr(val) )
    {
    case CVALUE:
      return cv_unbox( as_cvalue(val), size, spc );

    case RECORD ... AMT:
    case TYPE ... UPVALUE:
    case FIXNUM:
    case POINTER:
      if (size < 8)
	return 0;

      *(uintptr_t*)spc = (val&PTR_MASK);
      return 8;

    case NUL:
      if (size < 8)
	return 0;

      *(void**)spc = NULL;
      return 8;

    case BOOLEAN ... CTYPE:
    case UTF32:
    case SINT32 ... REAL32:
      if (size < 4)
	return 0;

      *(uint32_t*)spc = (uint32_t)(val&IMM_MASK);
      return 4;

    case OPCODE ... PRIMITIVE:
    case UTF16:
    case SINT16 ... UINT16:
      if (size < 2)
	return 0;

      *(uint16_t*)spc = (uint16_t)(val&IMM_MASK);
      return 2;

    case SINT8 ... UINT8:
    case ASCII ... UTF8:
      *(uint8_t*)spc = (uint8_t)(val&IMM_MASK);
      return 1;

    case REAL:
    default:
      if (size < 8)
	return 0;

      *(uintptr_t*)spc = val;
      return 8;
    }

  unreachable();
}

