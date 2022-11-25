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

/* runtime */
void rl_obj_atom_init( void );
void rl_obj_atom_mark( void );

/* convenience */
static inline atom_t *as_atom( value_t x ) { return (atom_t*)as_object(x); }
static inline bool    is_atom( value_t x ) { return rl_isa(x, &AtomType); }

#endif
