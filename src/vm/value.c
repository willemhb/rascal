#include "obj/object.h"
#include "obj/nul.h"
#include "obj/type.h"
#include "obj/stream.h"
#include "obj/real.h"

#include "vm/value.h"

/* API */
type_t rl_typeof( value_t x )
{
  switch ( tagof(x) )
    {
    case NUL:    return &NulType.data;
    case STREAM: return &StreamType.data;
    case OBJECT: return obj_head(as_object(x))->type;
    default:     return &RealType.data;
    }
}
