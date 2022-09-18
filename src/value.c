#include <stdio.h>

#include "value.h"
#include "object.h"
#include "number.h"
#include "table.h"

#define IMTYPE_MASK UINT16_MAX

ValueType valueType( Value x )
{
  Value tag = x&HEADER;

  switch (tag)
    {
    case IMMEDIATE: return x>> 32 & IMTYPE_MASK;
    case CHARACTER: return VAL_CHAR;
    case INTEGER:   return VAL_INT;
    case OBJECT:    return obj_type(x);
    case ARITY:     return TAG_ARITY;
    case POINTER:   return TAG_PTR;
    case CSTRING:   return TAG_CSTR;
    case HEADER:    return TAG_HEDR;
    default:        return VAL_REAL;
    }
}

Bool sameValues( Value x , Value y )
{
  return x == y;
}

Bool equalValues( Value x, Value y )
{
  return orderValues( x, y ) == 0;
}

Int orderValues( Value x, Value y )
{
  if (x == y)
    return 0;

  ValueType xType = valueType( x ), yType = valueType( y );

  if (xType != yType)
      return orderUInt( xType, yType );

  if (TypeDispatch[xType]->Order)
    return TypeDispatch[xType]->Order( x, y );

  return orderUInt( x, y );
}

Hash hashalue( Value x )
{
  ValueType xType = valueType( x );

  Hash base;

  if (TypeDispatch[xType]->Hash)
    base = TypeDispatch[xType]->Hash( x );

  else
    base = hashInt( x & ~HEADER );

  base = mixHash( base, xType );

  return (base&~HEADER) | ARITY; // compress to 48-bits and tag
}

Void printValue( Value x )
{
  ValueType xType = valueType( x );

  if (TypeDispatch[xType]->Print)
    TypeDispatch[xType]->Print( x );

  else
    printf( "<%s>", TypeDispatch[xType]->Name );
}
