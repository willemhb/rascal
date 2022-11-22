#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "rascal.h"

// core object types defined in here

#define OBJHEAD object_t obj

struct object_t
{
  type_t   *type;
  bool      allocated;
  uint      size;
};

struct type_t
{
  OBJHEAD;

  char        *name;
  size_t       size;
  value_t      constructor;
};

struct cons_t
{
  OBJHEAD;

  value_t car;
  value_t cdr;
};

// atom type definition
struct atom_t
{
  OBJHEAD;

  atom_t *left;
  atom_t *right;
  char   *name;
  value_t bind;     // toplevel binding
  value_t constant; // constant binding
};

// globals
extern atom_t *SymbolTable;

// forward declarations
// general constructor
object_t *make_obj( type_t *type );
void      free_obj( object_t *obj );
void      init_obj( object_t *obj, type_t *type, void *ini );

#define asType( x ) ((Type*)asObject(x))
#define isType( x ) valueIsType(x, &TypeType)

#define tagObj( x ) tagPtr( x, OBJTAG )

#endif
