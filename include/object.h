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
  uchar  repr     : 6;
  uchar  read     : 1;
  uchar  write    : 1;
  uchar  align;
  ushort width;
  uint   offset;
} fieldspec_t;

struct object_t
{
  object_t *next;           // live object list
  value_t   _meta;          // association list of object metadata
  type_t   *type;           // this object's type
  value_t   size      : 48;
  value_t   layout    :  3;
  value_t   gray      :  1;
  value_t   black     :  1;
  value_t   allocated :  1;
  value_t             :  2;
  value_t   flags     :  8;

  uchar     space[0];     // beginning of object's own data
};

typedef enum layout_t     // layout of a word/object
  {
   /* immediate layouts */
   real_layout      = 1,  // immediate, 64-bit float
   fixnum_layout    = 2,  // 48-bit unsigned integer (can represent anything)
   small_layout     = 3,  // 32-bit immediate data with additional type information

   /* common object layouts */
   record_layout    = 4,  // heterogenous but fixed-size object (arity field can be anything)
   array_layout     = 5,  // includes a variably-sized portion (arity field is the total allocated size)
  } layout_t;

struct type_t
{
  OBJHEAD;
  // size/layout information
  fieldspec_t *fields;
  size_t       n_fields;
  size_t       ob_size;
  size_t       el_size;     // arrays only
  layout_t     layout;

  string_t    *name;
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

  string_t *name;
  ulong     hash;
  atom_t   *left;
  atom_t   *right;
};

/* globals */
extern atom_t *SymbolTable;

// external
object_t *make_obj( type_t *type );
void      free_obj( object_t *obj );
void      init_obj( object_t *type, size_t n, void *ini );
void      obj_size( object_t *obj );

value_t   cons( value_t car, value_t cdr );
value_t   assoc( value_t key, value_t list );
value_t   cons_n( size_t n, ... );
value_t   list_n( size_t n, ... );

void      init_atom( atom_t *atom, char *name );
atom_t   *intern_atom( char *name );
value_t   atom( char *name );

#define tag_obj( x ) tag_ptr( x, OBJECT )

#define as_type( x ) ((type_t*)as_object(x))
#define is_type( x ) value_is_type(x, &TypeType)
#define as_cons( x ) ((cons_t*)as_object(x))
#define is_cons( x ) value_is_type(x, &ConsType)
#define as_atom( x ) ((atom_t*)as_object(x))
#define is_atom( x ) value_is_type(x, &AtomType)

#endif
