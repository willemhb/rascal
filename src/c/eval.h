#ifndef eval_h
#define eval_h

#include "common.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct Vm {
  chunk_t*  code;
  vector_t* envt;
  uint16*   ip;
  value_t*  sp;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern struct Vm Vm;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
value_t exec( chunk_t* chunk );
void repl( void );

#endif
