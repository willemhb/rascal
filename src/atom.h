#ifndef rascal_atom_h
#define rascal_atom_h

#include "table.h"

// C types --------------------------------------------------------------------

struct Atom
{
  OBJ_HEAD(UInt16);

  Table     *ns;
  String    *name;
  Hash       hash;
  Idno       idno;
};

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Atom *newAtom( const char *name, Table *ns );
Value internAtom( const char *name );

#endif
