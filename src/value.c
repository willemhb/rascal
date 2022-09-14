#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "object.h"


ValueType valueType( Value x )
{
  if (IS_NUMBER(x))
    return VAL_NUMBER;

  if (IS_NIL(x))
    return VAL_NIL;

  if (IS_BOOL(x))
    return VAL_BOOL;

  return VAL_OBJ;
}

bool ValuesSame( Value a, Value b )
{
  return a == b;
}

bool ValuesEqual( Value a, Value b )
{
  ValueType ta = valueType(a), tb = valueType(b);

  if (ta != tb)
    return false;

  if (ta == VAL_OBJ)
    {
      ObjType ota = OBJ_TYPE(ta), otb = OBJ_TYPE(tb);

      if (ota != otb)
	return false;

      if (ota == OBJ_STRING)
	{
	  ObjString *aString = AS_STRING(a), *bString = AS_STRING(b);

	  return aString->length == bString->length
	    && memcmp( aString->chars, bString->chars, aString->length  ) == 0;
	}
    }

  return ValuesSame( a, b );
}

void printValue( Value value )
{
  switch (valueType(value))
    {
    case VAL_NUMBER: break;
    case VAL_BOOL: printf( value == TRUE_VAL ? "true" : "false" ); break;
    case VAL_NIL: printf( "nil" ); break;
    case VAL_OBJ: printObject( value ); break;
    }
}
