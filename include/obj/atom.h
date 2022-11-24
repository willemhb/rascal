#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj/object.h"

/* C types */
struct atom_t
{
  OBJHEAD;

  string_t *name;
  ulong     hash;
  atom_t   *left;
  atom_t   *right;
};

/* globals */
extern type_t  AtomType;
extern atom_t *SymbolTable;

/* API */
value_t atom( char *name );

/* initialization */
void rl_obj_atom_init( void );

/* utilities & convenience */
#define as_atom( x ) ((atom_t*)as_object(x))
#define is_atom( x ) value_is_type(x, &AtomType)

#endif
