#include <stdarg.h>
#include <string.h>

#include "array.h"
#include "number.h"
#include "memory.h"

static const Size minCap = 8;

// general array utilities
Size padArray( Size newCount, Size oldCap )
{
  Size newCap = oldCap;

  while ( newCount < newCap/2 ) newCap >>= 1;
  while ( newCount > newCap ) newCap <<= 1;

  newCap = max(newCap, minCap);

  return newCap;
}

// buffer implemenation
Buffer *createBuffer( Void )
{
  return (Buffer*)create( &BufferType );
}

Void initBuffer( Buffer *created )
{
  created->count    = 0;
  created->capacity = minCap;
  created->data     = allocArray(created->capacity, sizeof(Char));
}

Buffer *newBuffer( Void )
{
  Buffer *created = createBuffer();
  initBuffer( created );
  return created;
}

Size resizeBuffer( Buffer *buf, Size newCount )
{
  Size oldCap = buf->capacity, newCap = padArray(newCount+1, oldCap);

  if ( oldCap != newCap )
    buf->data = reallocArray(buf->data, newCap, sizeof(Char));

  buf->count = newCount;
  buf->capacity = newCap;

  return newCap;
}

Void resetBuffer( Buffer *buf )
{
  resizeBuffer(buf, 0);
  memset(buf->data, 0, buf->capacity);
}

Size pushToBuffer( Buffer *buf, Char c )
{
  resizeBuffer(buf, buf->count+1);
  buf->data[buf->count-1] = c;
  
  return buf->count;
}

Size appendToBuffer( Buffer *dst, Size n, ... )
{
  Char buffer[n+1];

  va_list va; va_start(va, n);

  for (Size i=0; i<n; i++)
    buffer[i] = va_arg(va, Int);

  va_end(va);

  buffer[n] = '\0';

  return writeToBuffer(dst, buffer, n);
}

Size writeToBuffer( Buffer *dst, Char *src, Size n )
{
  Size oldCount =dst->count;
  
  resizeBuffer(dst, oldCount+n);
  strcpy(dst->data+oldCount, src);

  return dst->count;
}

Size readFromBuffer( Char *dst, Buffer *src, Size n )
{
  Size totalToWrite = min(n, src->count);
  strncpy(src->data, dst, totalToWrite);

  return totalToWrite;
}

// globals
Type BufferType =
  {
    {
      .dtype=&TypeType.obj
    },

    "buffer",
    sizeof(Buffer)
  };
