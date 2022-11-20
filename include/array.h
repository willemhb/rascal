#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// array types defined here, go in corresponding C file
// common array header
#define ARRAY					\
  OBJECT					\
  Size count;					\
  Size capacity;

// C types
typedef struct Buffer
{
  ARRAY
  Char *data;
} Buffer;

// implementation
Buffer     *createBuffer( Void );
Void        initBuffer( Buffer *created );
Buffer     *newBuffer( Void );

#define     isBuffer( x ) valueIsType(x, &BufferType)
#define     asBuffer( x ) ((Buffer*)asObject(x))

Size        resizeBuffer( Buffer *buf, Size newCount );
Void        resetBuffer( Buffer *buf );

Size        pushToBuffer( Buffer *buf, Char c );
Size        appendToBuffer( Buffer *buf, Size n, ... );
Size        writeToBuffer( Buffer *dst, Char *src, Size n );
Size        readFromBuffer( Char *dst, Buffer *src, Size n );

// globals
extern Type BufferType;

#endif
