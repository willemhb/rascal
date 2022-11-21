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

Value rlAscii( Ascii x )
{
  return tagValue(x, ASCIITAG);
}

Value rlPointer( Pointer x )
{
  return tagPtr(x, PTRTAG);
}

// utilities
ULong hashULong( ULong key )
{
    key = (~key) + (key << 21);            // key = (key << 21) - key - 1;
    key =   key  ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key =  key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key =  key ^ (key >> 28);
    key =  key + (key << 31);
    return key;
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
