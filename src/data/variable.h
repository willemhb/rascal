#ifndef data_variable_h
#define data_variable_h

#include "data/object.h"

// C types
struct variable {
  HEADER;
  symbol_t*    name;  // the unqualified name of the variable
  namespace_t* ns;    // namespace where the variable is defined
  table_t*     meta;  // other variable metadata
  value_t      bind;  // could be the binding itself, or the index of the binding (for lexical variables)
};

// APIs & utilities

#endif
