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

/* builtin types */
extern type_t TypeType, NulType, BoolType, RealType, PrimitiveType, ConsType,
  VectorType, InstructionsType, AtomType, LambdaType, ControlType, ClosureType,
  EnvironmentType;


// external
object_t *make_obj( type_t *type );
void      free_obj( object_t *obj );
void      init_obj( object_t *obj, type_t *type, void *ini );

cons_t   *make_cons( void );
void      init_cons( cons_t *cons, value_t car, value_t cdr );
void      free_cons( cons_t *cons );

atom_t   *make_atom( void );
void      init_atom( atom_t *atom, char *name );
void      free_atom( atom_t *atom );

atom_t   *intern_atom( char *name );

type_t   *make_type( void );
void      free_type( type_t *type );
void      init_type( type_t *type, char *name, size_t size, primitive_t constructor );

#define tag_obj( x ) tag_ptr( x, OBJTAG )

#define as_type( x ) ((type_t*)as_object(x))
#define is_type( x ) value_is_type(x, &TypeType)
#define as_cons( x ) ((cons_t*)as_object(x))
#define is_cons( x ) value_is_type(x, &ConsType)
#define as_atom( x ) ((atom_t*)as_object(x))
#define is_atom( x ) value_is_type(x, &AtomType)

#endif
