#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "array.h"
#include "atom.h"

// static helpers -------------------------------------------------------------
static bool equalCstrings( Cstring sx, Cstring sy )
{
  return strcmp( sx, sy ) == 0;
}

// atom helpers ---------------------------------------------------------------
Atom *newAtom( const char *name, bool isGenSym )
{
  size_t nChars = strlen(name);
  size_t total  = sizeof(Atom) + sizeof(String) + nChars + 1;
  char *space   = safeMalloc( __func__, total );

  Atom   *out    = (Atom*)space;
  String *nameOb = (String*)(space+sizeof(Atom));
  char   *chars  = space + sizeof(Atom) + sizeof (String);

  strcpy( chars, name );

  initString( nameOb, nChars, chars );
  initAtom( out, nameOb, isGenSym ? NULL : SymbolTable );

  return out;
}

void  initAtom( Atom *atom, String *name, AtomTable *ns )
{
  initObject( (Obj*)atom, OBJ_ATOM );

  atom->hash = mixHash( OBJ_ATOM, hashCstring( name->data ) );
  atom->name = name;
  atom->idno = SymbolCount++;
  atom->ns   = ns;
}

bool equalAtoms( Atom *a, Atom *b )
{
  return a == b;
}

hash_t hashAtom( Atom *a )
{
  return a->hash;
}

void printAtom( Atom *a )
{
  printf( ":%s", a->name->data );
}

Atom *internAtom( const char *name )
{
  AtomTableEntry *e;

  if (AtomTablePut(SymbolTable, (Cstring)name, &e))
    {
      Atom *new = newAtom( name, false );
      e->value = new;
    }

  return e->value;
}

// table implementation -------------------------------------------------------
TABLE_IMPL(AtomTable, Cstring, Atom*, AtomTableEntry, hashCstring, equalCstrings)

// initialization -------------------------------------------------------------
AtomTable SymbolTableObject;

void atomInit( void )
{
  SymbolTable = &SymbolTableObject;

  initAtomTable( SymbolTable );
  SymbolCount = 0;
}
