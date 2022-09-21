#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"
#include "template/table.h"


// C types --------------------------------------------------------------------
typedef struct atom_t
{
  OBJ_HEAD;
  obj_t     *name;
  hash_t     hash;
  idno_t     idno;
} atom_t;

typedef struct sym_table_t
{
  OBJ_HEAD;
  obj_t     *table;
  idno_t     counter;
} sym_table_t;

// globals --------------------------------------------------------------------
extern sym_table_t *SymbolTable;

// forward declarations -------------------------------------------------------
obj_t *intern( char *name );

#endif
