#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"

// describe macros ------------------------------------------------------------

// C types --------------------------------------------------------------------
typedef struct table_t
{
  HEADER;

  object_t **entries;
  uint cnt, cap;
} table_t;

typedef struct table_t dict_t;
typedef struct table_t set_t;
typedef struct table_t namespace_t;
typedef struct table_t symbol_table_t;

// globals --------------------------------------------------------------------
extern table_t *Symbols;
extern table_t *Signatures;

// forward declarations -------------------------------------------------------


#endif
