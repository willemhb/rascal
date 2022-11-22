#include <stdarg.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "table.h"


// general helpers
Object *create( Type *type )
{
  Size total = type->size;

  Object *out = allocate( total );
  out->dtype = &type->obj;

  return out;
}

// cons object implementation
Cons *makeCons( Void )
{
  return (Cons*)create( &ConsType );
}

Void initCons( Cons *created, Value car, Value cdr )
{
  created->car = car;
  created->cdr = cdr;
}

Int freeCons( Cons *cons )
{
  deallocate( cons, sizeof(Cons));
  return 0;
}

Value cons( Value car, Value cdr )
{
  Cons *out = makeCons();

  initCons( out, car, cdr );
  
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
Type *makeType( Void )
{
  return (Type*)create(&TypeType);
}

Void initType( Type *type, Char *name, Size size )
{
  type->name = name;
  type->size = size;
}

Int freeType( Type *type )
{
  deallocate(type, sizeof(Type));
  return 0;
}

// symbol implementation
Symbol *makeSymbol( Void )
{
  return (Symbol*)create(&SymbolType);
}

Void initSymbol( Symbol *symbol, Char *name, ULong hash )
{
  symbol->name  = strdup(name);
  symbol->hash  = hash;
  symbol->idno  = ++TheSymbolTable.counter;
  symbol->value = rlNul;
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

Type SymbolType =
  {
    {
      .dtype = &TypeType.obj
    },

    "symbol",
    sizeof(Symbol)
  };

Type TypeType =
  {
    {
      .dtype = &TypeType.obj,
    },

    "type",
    sizeof(Type)
  };
