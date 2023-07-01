#ifndef runtime_h
#define runtime_h

#include <setjmp.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum error {
  NO_ERROR,
  READ_ERROR,
  EVAL_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
};

enum token {
  TOKEN_READY,
  TOKEN_EXPRESSION,
  TOKEN_EOF,
  TOKEN_ERROR
};

struct vm {
  // main registers
  int ip, bp, fp, pp;

  values_t stack;

  // globals
  struct {
    table_t vars;
    values_t vals;
   } globals;

  // symbol table
  symbol_t* symbolTable;
  usize symbolCounter;

  // heap
  usize used, cap;
  bool managing;
  object_t* live;
  objects_t grays;

  // reader
  token_t token;
  port_t source;
  table_t dispatch;
  values_t expressions;
  buffer_t buffer;

  // error
  error_t error;
  value_t cause;
  char message[4096];
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern vm_t Vm;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// stack ----------------------------------------------------------------------
int push( value_t x );
value_t pop( void );
int pushn( int n );
value_t popn( int n );

// variables & methods --------------------------------------------------------
void toplevel_define( char* name, value_t bind );
value_t make_signature( usize n, ... );
void add_method( value_t func, value_t sig, value_t handler, flags fl );

// error ----------------------------------------------------------------------
void panic( void );
void recover( void );
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
void toplevel_init_runtime( void );

#endif
