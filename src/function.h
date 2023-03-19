#ifndef function_h
#define function_h

#include "value.h"

/* builtin types on which functions are built */
// C types --------------------------------------------------------------------
struct function_t {
  HEADER;
  rl_type_t* type;  // the type this function constructs (if it constructs a type)
  value_t template; // either a method table or a single method (if a single method, this method can't be overridden)
};

struct method_table_t {
  HEADER;
  dict_t* cache; // fast lookup (match exact argument signature)
  vector_t* farg;  // fixed arity methods
  list_t* varg;  // variable arity methods
};

struct method_t {
  HEADER;
  tuple_t* signature; // types to dispatch on
  value_t  handler; // value to apply when signature matches
};

// API ------------------------------------------------------------------------

#endif
