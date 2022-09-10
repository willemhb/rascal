#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "object.h"

// C types --------------------------------------------------------------------
struct symbol_t
{
  HEADER;
  object_t *name;
};

// macros ---------------------------------------------------------------------
// forward declarations -------------------------------------------------------

value_t symbol(char *name);


#endif
