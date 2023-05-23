#ifndef eval_h
#define eval_h

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct Vm {
  // frame variables (saved on non-tail call) ---------------------------------
  chunk_t*  code;
  vector_t* envt;
  uint16*   ip;
  value_t*  bp;
  value_t*  cp;
  value_t*  sp;

  // global variables ---------------------------------------------------------
  // toplevel environment -----------------------------------------------------
  table_t   toplevel;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern struct Vm Vm;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
value_t apply( void* head, usize n, value_t* args );
value_t exec( chunk_t* chunk );
void repl( void );

#endif
