#ifndef rascal_native_h
#define rascal_native_h

#include "obj/cvalue.h"

// native function pointer definitions and wrapper types for rascal-callable function signatures
typedef value_t (*thunk_t)( void );
typedef value_t (*unary_t)( value_t x );
typedef value_t (*binary_t)( value_t x, value_t y );
typedef value_t (*ternary_t)( value_t x, value_t y, value_t z );
typedef value_t (*nary_t)( value_t x, value_t y, value_t z );

typedef struct rl_thunk_t
{
  OBJECT

  thunk_t data;
} rl_thunk_t;

typedef struct rl_unary_t
{
  OBJECT

  thunk_t data;
} rl_unary_t;

typedef struct rl_binary_t
{
  OBJECT

  binary_t data;
} rl_binary_t;

typedef struct rl_ternary_t
{
  OBJECT

  ternary_t data;
} rl_ternary_t;

typedef struct rl_nary_t
{
  OBJECT

  nary_t data;
} rl_nary_t;

// globals
extern type_t ThunkType, UnaryType, BinaryType, TernaryType, NaryType;

// forward declarations


#endif
