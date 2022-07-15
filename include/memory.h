#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

bool managedp( value_t x );
bool overflowp( int n );

int  pow2resize( int new );
int  arr_resize( int new );

value_t relocate( value_t x );
void *allocate( size_t n );
void *reallocate( value_t ob, size_t b );

value_t forward( value_t x );

#endif
