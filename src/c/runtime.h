#ifndef runtime_h
#define runtime_h

#include <setjmp.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// execution state ------------------------------------------------------------
struct control {
  // main registers -----------------------------------------------------------
  int ip, bp, fp, fl;

  // stack state --------------------------------------------------------------
  value_t* sp;
};

// global variable state ------------------------------------------------------
struct toplevel {
  symbol_t* symbolTable;
  usize     symbolCounter;
  table_t   globalVars;
};

// heap state -----------------------------------------------------------------
struct heap {
  usize cap, used;
  object_t* live;
  vector_t  grays;
};

// reader state ---------------------------------------------------------------
typedef enum {
  READY_TOKEN,
  EXPRESSION_TOKEN,
  EOF_TOKEN,
  ERROR_TOKEN,
  INIT_TOKEN
} token_t;

struct reader {
  vector_t  expressions;
  binary_t  buffer;
  table_t   dispatch;
  port_t*   source;
  token_t   token;
};

// error statte ---------------------------------------------------------------
struct error {
  jmp_buf   buf;
  value_t   cause;
  char      message[2048];
};

// combined VM state ----------------------------------------------------------
struct vm {
  control_t  control;
  toplevel_t toplevel;
  heap_t     heap;
  reader_t   reader;
  error_t    error;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern vm_t Vm;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// stack ----------------------------------------------------------------------
int     push( value_t x );
value_t pop( void );
int     pushn( int n );
value_t popn( int n );

// error ----------------------------------------------------------------------
void error( const char* fname, value_t cause, const char* fmt, ... );
void require( const char* fname, bool test, value_t cause, const char* fmt, ... );
void forbid( const char* fname, bool test, value_t cause, const char* fmt, ... );

// memory ---------------------------------------------------------------------
void* allocate( usize nBytes, bool fromHeap );
void* reallocate( void* ptr, usize oldSize, usize newSize, bool fromHeap );
void* duplicate( void* ptr, usize nBytes, bool fromHeap );
char* duplicates( char* str, bool fromHeap );
void  deallocate( void* ptr, usize nBytes, bool fromHeap );

// misc -----------------------------------------------------------------------
void runtime_init( void );

#endif
