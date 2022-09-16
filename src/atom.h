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
  ARRAY_SPEC(AtomTableEntry*);
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
extern AtomTable *SymbolTable;
extern idno_t     SymbolCount;

// forward declarations -------------------------------------------------------
Atom *newAtom( const char *name, bool isGenSym );
void  initAtom( Atom *atom, String *name, AtomTable *ns );
Atom *internAtom( const char *name );

bool   equalAtoms( Atom *a, Atom *b );
hash_t hashAtom( Atom *a );

TABLE_API(AtomTable, Cstring, Atom*, AtomTableEntry);

void atomInit( void );

#endif
