#ifndef rascal_atom_h
#define rascal_atom_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef uintptr_t hash_t;

struct Atom
{
  OBJ_HEAD;
  Map       *ns;
  String    *name;
  hash_t     hash;
  idno_t     idno;
};

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Atom *newAtom( const char *name, bool isGenSym );
void  initAtom( Atom *atom, String *name, Map *ns );
Value internAtom( const char *name );

bool   equalAtoms( Value, Value b );
hash_t hashAtom( Value a );
void   printAtom( Value a );

#endif
