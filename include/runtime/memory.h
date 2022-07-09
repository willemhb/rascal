#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

typedef enum {
  memfl_global = 0x01
} memory_flags_t;

// exports --------------------------------------------------------------------
bool     globalp( void *p );
bool     overflowp( size_t n );
size_t   arr_resize( size_t n );
size_t   pow2_resize( size_t n );
void    *getword( void *p );
size_t   getoff( void *p );

// allocation procedures ------------------------------------------------------
void     *allocate( flags_t fl , size_t n );
object_t *reallocate( object_t *x, size_t o, size_t n );
void      manage( void );

#endif
