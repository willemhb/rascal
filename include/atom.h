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

// macros & statics -----------------------------------------------------------


#endif
