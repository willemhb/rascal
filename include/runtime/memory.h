#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

enum memflags_t {
  memfl_global = 0x01
};

// exports --------------------------------------------------------------------
bool     globalp( void *p );
bool     overflowp( int n );
int      arr_resize( int n );
int      pow2_resize( int n );
void    *getword( void *p );
int      getoffset( void *p );

// allocation procedures ------------------------------------------------------
void     *allocate( flags_t fl , int n );
object_t *reallocate( object_t *x, int o, int n );
value_t   copy( value_t x, int n );

// GC entry points ------------------------------------------------------------
void manage( void );

value_t relocate( value_t x );
void    trace( void *p, );

#endif
