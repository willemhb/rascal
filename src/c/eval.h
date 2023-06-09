#ifndef eval_h
#define eval_h

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct frame_t frame_t;

struct frame_t {
  frame_t* cp,* fp;
  chunk_t* code;

  union {
    vector_t* envt;
    value_t*  bp;
  };

  value_t* sp;
  uint16*  ip;
};

struct Vm {
  frame_t  frame;
  table_t  global;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern struct Vm Vm;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
value_t exec( chunk_t* chunk );
void    repl( void );

// misc _______________________________________________________________________
bool is_literal( value_t x );
void vm_init( void );

#endif
