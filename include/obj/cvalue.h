#ifndef rascal_obj_cvalue_h
#define rascal_obj_cvalue_h

#include "obj/object.h"

// immediate and primitive types and primitive types that wrap C values.
// includes numbers, characters, and wrapped pointer types.
// metaobject types

// indicates the C type of the value and how it's boxed
typedef enum repr_t
  {
    SINT8, UINT8, ASCII, LATIN1, UTF8,

    SINT16, UINT16, UTF16, OPCODE, PRIMITIVE,

    SINT32, UINT32, UTF32, REAL32, CTYPE, BOOLEAN,

    NUL, FIXNUM, PTR, REAL, OBJ,
  } repr_t;

#define N_IMM  ( FIXNUM )
#define N_REPR ( OBJ+1 )

typedef rl_value_t  (*unwrap_t)( value_t val );
typedef value_t     (*wrap_t)( type_t *type, rl_value_t x );
typedef int         (*unbox_t)( void *dst, value_t src );

// implementer for 
typedef struct cvalue_impl_t
{
  OBJECT;

  Ctype_t  Ctype; // C type of the boxed or wrapped value
  arity_t  size;  // size of the boxed or wrapped data

  wrap_t   wrap;
  unwrap_t unwrap;
  unbox_t  unbox;
} cvalue_impl_t;

// forward declarations
rl_value_t rl_unwrap( value_t val );
rl_value_t fixnum_unwrap( value_t val );
rl_value_t real_unwrap( value_t val );
rl_value_t imm_unwrap( value_t val );
rl_value_t cv8_unwrap( value_t val );
rl_value_t cv16_unwrap( value_t val );
rl_value_t cv32_unwrap( value_t val );
rl_value_t cv64_unwrap( value_t val );

value_t rl_wrap( type_t *type, rl_value_t x );
value_t fixnum_wrap( type_t *type, rl_value_t x );
value_t nul_wrap( type_t *type, rl_value_t x );
value_t real_wrap( type_t *type, rl_value_t x );
value_t imm_wrap( type_t *type, rl_value_t x );
value_t cv8_wrap( type_t *type, rl_value_t x );
value_t cv16_wrap( type_t *type, rl_value_t x );
value_t cv32_wrap( type_t *type, rl_value_t x );
value_t cv64_wrap( type_t *type, rl_value_t x );

int rl_unbox( void *dst, value_t val );
int fixnum_unbox( void *dst, value_t val );
int nul_unbox( void *dst, value_t val );
int real_unbox( void *dst, value_t val );
int imm8_unbox( void *dst, value_t val );
int imm16_unbox( void *dst, value_t val );
int imm32_unbox( void *dst, value_t val );
int cv8_unbox( void *dst, value_t val );
int cv16_unbox( void *dst, value_t val );
int cv32_unbox( void *dst, value_t val );
int cv64_unbox( void *dst, value_t val );

// globals
// primitive & immediate types that don't fit elsewhere
extern type_t CvalueImplType, BooleanType, OpcodeType, PrimitiveType, PointerType;

// array of immediate types (for looking up type by code)
extern type_t *ImmTypes[N_IMM];

// convenience
#define cvalue_type_ctype( x )  ( type_cvalue_impl( x )->Ctype )
#define cvalue_type_size( x )   ( type_cvalue_impl( x )->size )
#define cvalue_type_wrap( x )   ( type_cvalue_impl( x )->wrap )
#define cvalue_type_unwrap( x ) ( type_cvalue_impl( x )->unwrap )
#define cvalue_type_unbox( x )  ( type_cvalue_impl( x )->unbox )

#endif
