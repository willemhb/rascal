#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    atom_fl_gensym =1,
    atom_fl_keyword=2,
  } atom_fl_t;

struct symbol_t
{
  HEADER;
  object_t *name;
  ulong     hash;
  ulong     idno;
};

// forward declarations -------------------------------------------------------
value_t symbol(char *name);
value_t gensym(char *name);

// globals --------------------------------------------------------------------
extern table_t *Symbols;
extern ulong    SymbolCounter;

// macros & statics -----------------------------------------------------------
#define as_symbol(x) asa(symbol_t*, x, pval)

#define sym_name(x)  getf(symbol_t*, x, name)
#define sym_hash(x)  getf(symbol_t*, x, hash)
#define sym_idno(x)  getf(symbol_t*, x, idno)

#endif
