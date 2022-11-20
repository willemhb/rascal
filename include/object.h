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

typedef struct Port
{
  OBJECT

  FILE *stream;
} Port;

typedef struct Type
{
  OBJECT

  Char *name;
  Size  size;
} Type;

// globals
extern Type ConsType, PortType, TypeType;

// forward declarations
// general constructor
Object *create( Type *type );

#define tagObj( x ) tagPtr( x, OBJTAG )

// cons api
Cons *createCons( Void );
Void  initCons( Cons *created, Value car, Value cdr );
Cons *newCons( Value car, Value cdr );
Value cons( Value car, Value cdr );
Value consn( Size len, ... );

#define listn( len, ... ) consn((len), __VA_ARGS__ __VA_OPT__(,) rlNul)

#define asCons( x ) ((Cons*)asObject(x))
#define isCons( x ) valueIsType( x, &ConsType )

// port api
Port *createPort( Void );
Void  initPort( Port *created, FILE *stream );
Port *newPort( FILE *stream );

#define asPort( x ) ((Port*)asObject(x))
#define isPort( x ) valueIsType(x, &PortType)

// type api
Type *createType( Void );
Void  initType( Type *created, Char *name, Size size );
Type *newType( Char *name, Size size );

#define asType( x ) ((Type*)asObject(x))
#define isType( x ) valueIsType(x, &TypeType)

#endif
