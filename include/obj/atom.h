#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj/object.h"

/* C types */
struct atom_t
{
  OBJHEAD;

  char     *name;
  ulong     hash;
  ulong     idno;
  atom_t   *left;
  atom_t   *right;
};

/* globals */
extern type_t  AtomType;
extern atom_t *SymbolTable;

/* API */
value_t atom( char *name );
value_t gensym( char *name );

/* runtime */
void rl_obj_atom_init( void );
void rl_obj_atom_mark( void );

/* convenience */
#define as_atom( x ) ((atom_t*)as_object(x))

static inline bool    is_atom( value_t x ) { return rl_isa(x, &AtomType); }

#endif
