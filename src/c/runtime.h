#ifndef runtime_h
#define runtime_h

#include <setjmp.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// memory state ---------------------------------------------------------------
struct Heap {
  usize cap, used;
  object_t* live;
  vector_t grays;
};

// error state ----------------------------------------------------------------
struct Error {
  value_t cause;
  jmp_buf safety;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern struct Heap Heap;
extern struct Error Error;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// error ----------------------------------------------------------------------
void panic( value_t cause );
void error( const char* fname, value_t cause, const char* fmt, ...);
void require( const char* fname, bool test, value_t cause, const char* fmt, ...);
void forbid( const char* fname, bool test, value_t cause, const char* fmt, ...);

// memory ---------------------------------------------------------------------
void* allocate( usize nBytes );
void* reallocate( void* ptr, usize oldSize, usize newSize );
void* duplicate( void* ptr, usize nBytes );
char* duplicate_str( char* str );
void  deallocate( void* ptr, usize nBytes );

// misc -----------------------------------------------------------------------
void runtime_init( void );

#endif
