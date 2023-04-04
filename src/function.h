#ifndef function_h
#define function_h

#include "value.h"

/* builtin types on which functions are built */
// C types --------------------------------------------------------------------
struct function_t {
  HEADER;
  function_t* next;       // next method to try
  table_t*    cache;      // method signatures cached by exact signature
  vector_t*   signature;  // argument signature
  type_t*     type;       // type constructed by the function (if any)
  value_t     template;   // native (pointer), compiled (closure), or primitive (fixnum)
};

struct chunk_t {
  HEADER;
  alist_t*  constants;
  list_t*   namespace;
  buffer_t* bytecode;
};

struct closure_t {
  HEADER;
  chunk_t* code;
  alist_t* upvalues;
};

#endif
