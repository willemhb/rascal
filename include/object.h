#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "rascal.h"

// core object types defined in here

#define OBJHEAD object_t obj

typedef enum repr_t
  {
    repr_Cdata  =1,
    repr_Cptr   =2,
    repr_Cstring=3,
    repr_value  =4,
    repr_object =5,
    repr_values =6,
    repr_objects=7,
  } repr_t;

typedef struct fieldspec_t
{
  uchar  repr     : 3;
  uchar  free     : 1;
  uchar  finalize : 1;
  uchar  trace    : 1;
  uchar  read     : 1;
  uchar  write    : 1;
  uchar  align;
  ushort width;
  uint   offset;
} fieldspec_t;

struct object_t
{
  object_t *next;         // live object list
  type_t   *type;         // this object's type
  value_t   _meta;        // association list of object metadata
  uint      size;         // total object size, including header and associated data
  uchar     allocated;    // whether the object is allocated (many core Vm objects are not)
  uchar     gray;         // GC gray bit
  uchar     black;        // GC black bit
  uchar     flags;        // discretionary flags

  uchar     space[0];     // beginning of object's own data
};

struct type_t
{
  OBJHEAD;

  char        *name;
  size_t       size;
  value_t      constructor;
  size_t       n_fields;
  fieldspec_t *fields;
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

  char   *name;
  ulong   hash;
  atom_t *left;
  atom_t *right;
};

/* globals */
extern atom_t *SymbolTable;

// external
object_t *make_obj( type_t *type );
void      free_obj( object_t *obj );

void      init_cons( cons_t *cons, value_t car, value_t cdr );
value_t   cons( value_t car, value_t cdr );
value_t   assoc( value_t key, value_t list );
value_t   cons_n( size_t n, ... );
value_t   list_n( size_t n, ... );

void      init_atom( atom_t *atom, char *name );
atom_t   *intern_atom( char *name );
value_t   atom( char *name );

void      init_type( type_t *type, char *name, size_t size, primitive_t constructor );

#define tag_obj( x ) tag_ptr( x, OBJECT )

#define as_type( x ) ((type_t*)as_object(x))
#define is_type( x ) value_is_type(x, &TypeType)
#define as_cons( x ) ((cons_t*)as_object(x))
#define is_cons( x ) value_is_type(x, &ConsType)
#define as_atom( x ) ((atom_t*)as_object(x))
#define is_atom( x ) value_is_type(x, &AtomType)

#endif
