#include "obj/object.h"
#include "obj/stream.h"
#include "obj/real.h"

#include "vm/value.h"

/* API */
struct type_t *rl_typeof( value_t x )
{
  switch (tagof(x))
    {
    case STREAM: return &StreamType;
    case OBJECT: return obj_head(as_object(x))->type;
    default:     return &RealType;
    }
}
