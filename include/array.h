#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// array types defined here, go in corresponding C file

// common array header
#define ARRAY(V)				\
  OBJHEAD;					\
  size_t  count;				\
  size_t  capacity;				\
  V      *data

// C types
struct vector_t
{
  OBJHEAD;

  size_t   count;
  size_t   capacity;
  value_t *data;
};

// implementations
// buffer

// vector


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
