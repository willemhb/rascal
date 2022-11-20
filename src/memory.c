#include <stdlib.h>
#include <string.h>

#include "memory.h"


Void *allocate( Size nBytes )
{
  Void *out = alloc_s( malloc, nBytes );
  memset( out, 0, nBytes );
  return out;
}


Void *reallocate( Void *ptr, Size nBytes )
{
  Void *out = alloc_s( realloc, ptr, nBytes );
  return out;
}


Int deallocate( Void *ptr )
{
  free(ptr);

  return 0;
}
