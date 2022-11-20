#include "number.h"
#include "object.h"

// implementations
Value rlFixnum( Fixnum x )
{
  return tagValue(x, FIXTAG);
}

Value rlReal( Real x )
{
  return asValue( x );
}

// globals
Type RealType =
  {
    {
      .dtype = &TypeType.obj
    },

    "real",
    sizeof(Real)
  };

Type NulType =
  {
    {
      .dtype = &TypeType.obj
    },

    "nul",
    sizeof(Void*)
  };

Type FixnumType =
  {
    {
      .dtype = &TypeType.obj
    },

    "fixnum",
    sizeof(Fixnum)
  };

Type *ImmediateTypes[] =
  {
    &NulType
  };
