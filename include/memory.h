#ifndef rascal_memory_h
#define rascal_memory_h

#include "value.h"

// memory api, eventual GC implementation
Void *allocate( Size nBytes );
Void *reallocate( Void *ptr, Size nBytes );
Int   deallocate( Void *ptr );

#endif
