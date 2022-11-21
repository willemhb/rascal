#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "value.h"

// core object types are defined here
typedef struct Cons
{
  OBJECT

  Value car;
  Value cdr;
} Cons;

typedef struct Type
{
  OBJECT

  Char *name;
  Size  size;
} Type;

// globals
extern Type ConsType, TypeType;

// forward declarations
// general constructor
Object *create( Type *type );

#define tagObj( x ) tagPtr( x, OBJTAG )

// cons api
Cons *createCons( Void );
Int   freeCons( Cons *cons );
Void  initCons( Cons *created, Value car, Value cdr );

Value cons( Value car, Value cdr );
Value consn( Size len, ... );

#define listn( len, ... ) consn((len)+1, __VA_ARGS__ __VA_OPT__(,) rlNul)

#define asCons( x ) ((Cons*)asObject(x))
#define isCons( x ) valueIsType( x, &ConsType )

// type api
Type *createType( Void );
Int   freeType( Type *type );
Void  initType( Type *created, Char *name, Size size );

#define asType( x ) ((Type*)asObject(x))
#define isType( x ) valueIsType(x, &TypeType)

#endif
