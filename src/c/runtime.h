#ifndef runtime_h
#define runtime_h

#include <setjmp.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum token {
  TOKEN_READY,
  TOKEN_EXPRESSION,
  TOKEN_EOF,
  TOKEN_ERROR
};

struct vm {
  // currently executing frame (main registers)
  frame_t frame;

  // stack pointers
  int sp, fp;

  // toplevel environment
  table_t globals;

  // symbol table
  symbol_t* symbolTable;
  usize symbolCounter;

  // heap
  usize used, cap;
  bool managing, initialized;
  object_t* live;
  alist_t grays;

  // reader
  token_t  token;
  port_t   source;
  table_t  dispatch;
  alist_t  expressions;
  buffer_t buffer;

  // error
  jmp_buf context;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// pseudo-registers -----------------------------------------------------------
#define SP      (Vm.sp)
#define FP      (Vm.fp)
#define FRAME   (Vm.frame)
#define FN      (Vm.frame.fn)
#define IP      (Vm.frame.ip)
#define BP      (Vm.frame.bp)
#define FL      (Vm.frame.fl)
#define ENV     (Values[BP])
#define TOS     (Values[SP-1])
#define VALS    (Vm.frame.fn->vals->data)

extern vm_t Vm;
extern value_t Values[]; // values stack
extern frame_t Frames[]; // frames stack

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// toplevel reset functions ---------------------------------------------------
void reset_interp( void );
void reset_reader( void );

// stack ----------------------------------------------------------------------
value_t* push( value_t val );
value_t  pop( void );
value_t  popnth( int n );
value_t  popn( int n );

// frame ----------------------------------------------------------------------
void push_frame( void );
void pop_frame( void );

// error ----------------------------------------------------------------------
void error( const char* fname, value_t cause, const char* fmt, ... );
void require( const char* fname, bool test, value_t cause, const char* fmt, ... );
void forbid( const char* fname, bool test, value_t cause, const char* fmt, ... );
void argco( const char* fname, bool variadic, usize expect, usize got );

// memory ---------------------------------------------------------------------
void push_gray( void* o );
void* pop_gray( void );
void* allocate( usize nBytes, bool fromHeap );
void* reallocate( void* ptr, usize oldSize, usize newSize, bool fromHeap );
void* duplicate( void* ptr, usize nBytes, bool fromHeap );
char* duplicates( char* str, bool fromHeap );
void  deallocate( void* ptr, usize nBytes, bool fromHeap );

// misc -----------------------------------------------------------------------
void toplevel_init_runtime( void );

#endif
