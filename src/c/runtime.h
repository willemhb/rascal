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

struct frame {
  chunk_t* fn;
  ushort* ip;
  value_t* bp;
  tuple_t* env;
};

struct vm {
  // stack pointers
  value_t* sp;
  frame_t* fp;

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
// pseudo-registers -----------------------------------------------------------
#define SP  (Vm.sp)
#define FP  (Vm.fp)
#define FN  (FP->fn)
#define IP  (FP->ip)
#define BP  (FP->bp)
#define ENV (FP->env)

extern vm_t Vm;
extern value_t Values[]; // values stack
extern frame_t Frames[]; // frames stack

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// stack ----------------------------------------------------------------------
value_t* push( value_t val );
value_t pop( void );
bool in_stack( void* ptr );

// frame ----------------------------------------------------------------------
frame_t* push_frame( void );
void pop_frame( void );
tuple_t* capture_frame( frame_t* f );

// variables & methods --------------------------------------------------------
void toplevel_define( char* name, value_t bind );
value_t toplevel_lookup( value_t name );

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
