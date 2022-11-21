#include <stdarg.h>

#include "object.h"
#include "memory.h"


// general helpers
Object *create( Type *type )
{
  Size total = type->size;

  Object *out = allocate( total );
  out->dtype = &type->obj;

  return out;
}

// cons object implementation
Cons *createCons( Void )
{
  return (Cons*)create( &ConsType );
}

Void initCons( Cons *created, Value car, Value cdr )
{
  created->car = car;
  created->cdr = cdr;
}

Cons *newCons( Value car, Value cdr )
{
  
  Cons* created = createCons();
  initCons( created, car, cdr );

  return created;
}

Value cons( Value car, Value cdr )
{
  Cons *out = newCons( car, cdr );
  
  return tagObj( out );
}

Value consn( Size len, ... )
{
  va_list va; va_start(va, len);

  Value buffer[len];

  for (Size i=0; i<len; i++)
    buffer[i] = va_arg(va, Value);

  va_end(va);

  Value out = buffer[len-1];

  for (Size i=len-1; i>0; i--)
    out = cons(buffer[i-1], out);

  return out;
}

// type object implementation
Type *createType( Void )
{
  return (Type*)create( &TypeType );
}

Void initType( Type *created, Char *name, Size size )
{
  created->name = name;
  created->size = size;
}

Type *newType( Char *name, Size size )
{
  Type *created = createType();
  initType( created, name, size );
  return created;
}

// globals (mostly metaobjects)

Type ConsType =
  {
    {
      .dtype = &TypeType.obj
    },

    "cons",
    sizeof(Cons)
  };

Type PortType =
  {
    {
      .dtype = &TypeType.obj
    },

    "port",
    sizeof(Port)
  };

Type TypeType =
  {
    {
      .dtype = &TypeType.obj,
    },

    "type",
    sizeof(Type)
  };
