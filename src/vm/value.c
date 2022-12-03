#include "obj/real.h"
#include "obj/fixnum.h"
#include "obj/glyph.h"
#include "obj/bool.h"
#include "obj/nul.h"
#include "obj/stream.h"
#include "obj/native.h"
#include "obj/type.h"

#include "vm/object.h"
#include "vm/value.h"

/* API */
datatype_t *rl_typeof( value_t x )
{
  switch ( tagof(x) )
    {
    case FIXNUM: return &FixnumType;
    case BOOL:   return &BoolType;
    case NUL:    return &NulType;
    case STREAM: return &StreamType;
    case NATIVE: return &NativeType;
    case OBJECT: return as_object(x)->type;
    default:     return &RealType;
    }
}

bool rl_isa( value_t x, type_t *type )
{
  return type->isa(type, x);
}

void mark_value( value_t x )
{
  if ( is_object(x) )
    mark_object(as_object(x));
}

void *toptr( value_t x )
{
  if ( x == NUL )
    return NULL;

  return (void*)(x&PTRMASK);
}

/* runtime */
void rl_vm_value_init( void )    {}
void rl_vm_value_mark( void )    {}
void rl_vm_value_cleanup( void ) {}
