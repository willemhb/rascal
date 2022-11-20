#include "value.h"
#include "object.h"
#include "number.h"

Type *rlTypeof( Value x )
{
  switch (x&TAGMASK)
    {
    case FIXTAG: return &FixnumType;
    case OBJTAG: return (Type*)asObject(x)->dtype;
    case IMMTAG: return ImmediateTypes[(x>>32)&255];
    default:     return &RealType;
    }
}
