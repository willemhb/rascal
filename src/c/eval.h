#ifndef eval_h
#define eval_h

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct frame_t frame_t;

struct frame_t {
  frame_t* cp,* fp;
  chunk_t* code;
  value_t* bp,* sp;
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
value_t apply( void* head, usize n, value_t* args );
value_t exec( chunk_t* chunk );
void repl( void );

// misc _______________________________________________________________________
void vm_init( void );

#endif
