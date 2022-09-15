#ifndef rascal_atom_h
#define rascal_atom_h

#include "table.h"

// C types --------------------------------------------------------------------
typedef uintptr_t hash_t;

typedef struct
{
  ENTRY_SPEC(Cstring, Atom*);
} AtomTableEntry;

typedef struct
{
  ARRAY_SPEC(AtomTableEntry);
} AtomTable;

struct Atom
{
  OBJ_HEAD;
  AtomTable *ns;
  String    *name;
  hash_t     hash;
  idno_t     idno;
};

// globals --------------------------------------------------------------------


// forward declarations -------------------------------------------------------
extern Atom *intern( const char *name );

#endif
