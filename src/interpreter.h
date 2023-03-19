#ifndef interpreter_h
#define interpreter_h

#include "value.h"

/* builtin types used predominantly by the compiler and virtual machine */

// C types --------------------------------------------------------------------
struct chunk_t {
  HEADER;
  namespace_t* ns;
  tuple_t* vals;
  binary_t* code;
};

struct closure_t {
  HEADER;
  chunk_t* chunk;
  environment_t* envt;
};

struct variable_t {
  HEADER;
  value_t     name;
  value_t     doc;
  value_t     type;
  value_t     bind;
  variable_t* next; // for upvalues
};

struct namespace_t {
  HEADER;
  dict_t*      locals;
  dict_t*      upvalues;
  namespace_t* next;
};

struct environment_t {
  HEADER;
  usize len, cap;
  variable_t** upvals;
};

struct control_t {
  HEADER;
  control_t* caller;
  closure_t* func;
  uint16*    ip;
  tuple_t*   rxs;
};

// API ------------------------------------------------------------------------


#endif
