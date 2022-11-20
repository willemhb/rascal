#include <stdlib.h>
#include <string.h>

#include "memory.h"

// safe allocation functions
Void *allocate( Size nBytes )
{
  Void *out = alloc_s( malloc, nBytes );
  memset( out, 0, nBytes );
  return out;
}

Void *allocArray( Size count, Size elSize )
{
  Void *out = alloc_s( calloc, count, elSize );
  memset( out, 0, count*elSize );
  return out;
}

Void *reallocate( Void *ptr, Size nBytes )
{
  Void *out = alloc_s( realloc, ptr, nBytes );
  return out;
}

Void *reallocArray( Void *ptr, Size count, Size elSize )
{
  return reallocate( ptr, count*elSize );
}

Int deallocate( Void *ptr )
{
  free(ptr);

  return 0;
}
