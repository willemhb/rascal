
#include "obj/nul.h"
#include "obj/type.h"
#include "obj/stream.h"
#include "obj/real.h"


#include "vm/object.h"
#include "vm/value.h"

/* API */
datatype_t *rl_typeof( value_t x )
{
  switch ( tagof(x) )
    {
    case NUL:    return &NulType;
    case STREAM: return &StreamType;
    case OBJECT: return as_object(x)->type;
    default:     return &RealType;
    }
}

void mark_value( value_t x )
{
  if ( is_object(x) )
    mark_object(as_object(x));
}

/* runtime */
void rl_vm_value_init( void ) {}
void rl_vm_value_mark( void ) {}
