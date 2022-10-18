#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "obj/type.h"

typedef struct
{
  OBJECT
  bool   gensym;
  bool   keyword;
  idno_t idno;
  hash_t hash;
  char  *name;
} symbol_t;

typedef struct
{
  OBJECT

  object_t **symbols;
  size_t     length;
  size_t     capacity;
  idno_t     counter;
} symbols_t;

// globals
extern symbols_t Symbols;

// forward declarations


#endif
