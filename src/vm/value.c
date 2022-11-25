#include "vm/value.h"
#include "obj/cons.h"    // for NulType
#include "obj/bool.h"
#include "obj/glyph.h"
#include "obj/fixnum.h"
#include "obj/smint.h"
#include "obj/native.h"
#include "obj/stream.h"
#include "obj/object.h"
#include "obj/real.h"

/* API */
type_t *rl_typeof( value_t x )
{
  switch ( get_tag(x) )
    {
    case NUL:     return &NulType;
    case BOOLEAN: return &BoolType;
    case GLYPH:   return &GlyphType;
    case FIXNUM:  return &FixnumType;
    case SMINT:   return &SmintType;
    case NATIVE:  return &NativeType;
    case STREAM:  return &StreamType;
    case OBJECT:  return as_object(x)->type;
    default:      return &RealType;
    }
}

size_t rl_sizeof( value_t x )
{
  switch ( get_tag(x) )
    {
    case NUL:     return sizeof(nul_t);
    case BOOLEAN: return sizeof(bool_t);
    case GLYPH:   return sizeof(glyph_t);
    case FIXNUM:  return sizeof(fixnum_t);
    case SMINT:   return sizeof(smint_t);
    case NATIVE:  return sizeof(native_fn_t);
    case STREAM:  return sizeof(stream_t*);
    case OBJECT:  return as_object(x)->size;
    default:      return sizeof(real_t);
    }
}


bool is_leaf( value_t x )
{
  if ( is_object(x) )
    return flagp(as_object(x)->flags, mem_is_leaf);

  return true;
}
