#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

typedef enum {
  memfl_global = 0x01
} memory_flags_t;

// exports --------------------------------------------------------------------
bool     globalp( void *p );
bool     overflowp( int n );
int      arr_resize( int n );
int      pow2_resize( int n );
void    *getword( void *p );
int      getoff( void *p );

// allocation procedures ------------------------------------------------------
void     *allocate( flags_t fl , int n );
value_t   reallocate( value_t x, int o, int n );
void      manage( void );

#endif
