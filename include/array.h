#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// array types defined here, go in corresponding C file
// common array header
#define ARRAY(eltype)				\
  OBJECT					\
  Size count;					\
  Size capacity;				\
  eltype *data

// C types
typedef struct Buffer Buffer;
typedef struct Instr  Instr;
typedef struct Vector Vector;

struct Buffer
{
  ARRAY(Char);
};

struct Instr
{
  ARRAY(UShort);
};

struct Vector
{
  ARRAY(Value);
};

// implementations
// buffer
Buffer *makeBuffer( Void );
Int     freeBuffer( Buffer *buffer );
Void    initBuffer( Buffer *created );
Size    resizeBuffer( Buffer *buffer, Size newCount );

Size    appendToBuffer( Buffer *buffer, Size n, ... );
Char    getFromBuffer( Buffer *buffer, Long i );
Buffer *setInBuffer( Buffer *buffer, Long i, Char c );
Char    popFromBuffer( Buffer *buffer, Size n );

Size    writeToBuffer( Buffer *dst, Char *src, Size n );
Size    readFromBuffer( Char *dst, Buffer *src, Size n );

#define isBuffer( x )    valueIsType(x, &BufferType)
#define asBuffer( x )    ((Buffer*)asObject(x))
#define resetBuffer( x ) resizeBuffer(x, 0)

// instr
Instr  *makeInstr( Void );
Int     freeInstr( Instr *instr );
Void    initInstr( Instr *instr );
Size    resizeInstr( Instr *instr, Size newCount );

Size    appendToInstr( Instr *instr, Size n, ... );
UShort  getFromInstr( Instr *instr, Long i );
Instr  *setInInstr( Instr *instr, Long i, UShort op );
UShort  popFromInstr( Instr *instr, Size n );

#define isInstr( x ) valueIsType(x, &InstrType)
#define asInstr( x ) ((Instr*)asObject(x))

// vector
Vector *makeVector( Void );
Int     freeVector( Vector *vector );
Void    initVector( Vector *vector );
Size    resizeVector( Vector *vector, Size newCount );

Size    appendToVector( Vector *vector, Size n, ... );
Value   getFromVector( Vector *vector, Long i );
Vector *setInVector( Vector *vector, Long i, Value value );
Value   popFromVector( Vector *vector, Size n );

// globals
extern Type BufferType, InstrType, VectorType;

// convenience
#define aref( x, n )				\
  _Generic((x),					\
	   Buffer*:getFromBuffer,		\
	   Instr*:getFromInstr,			\
	   Vector*:getFromVector)((x), (n))

#define aset( a, i, x )				\
  _Generic((x),					\
	   Buffer*:setInBuffer,			\
	   Instr*:setInInstr,			\
	   Vector*:setInVector)((a), (i), (x))

#define apush( x, n, ... )						\
  _Generic((x),								\
	   Buffer*:appendToBuffer,					\
	   Instr*:appendToInstr,					\
	   Vector*:appendToVector)((x), (n) __VA_OPT__(,) __VA_ARGS__)

#define apop( x, n )							\
  _Generic((x),								\
	   Buffer*:popFromBuffer,					\
	   Instr*:popFromInstr,						\
	   Vector*:popFromVector)((x), (n))

#endif
