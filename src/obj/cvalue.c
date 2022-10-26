#include "obj/cvalue.h"
#include "obj/object.h"

// toplevel dispatch for wrapping/unwrapping
rl_value_t rl_unwrap( value_t val )
{
  type_t *type = val_type( val );

  
  return cvalue_type_unwrap( val_type( val ) )( val );
}

value_t rl_wrap( type_t *type, rl_value_t x )
{
  return dtype_cvalue_impl( type )->wrap( type, x );
}

int rl_unbox( void *spc, value_t val )
{
  return val_type( val )->dtype->unbox( spc, val );
}

// unwrap implementations
#define cv_unwrap( size )					\
  rl_value_t cv##size##_unwrap( value_t val )			\
  {								\
    return (rl_value_t)(*(uint##size##_t*)(as_obj(val)+1));	\
  }

rl_value_t fixnum_unwrap( value_t val )
{
  return (rl_value_t)(val&PTR_MASK);
}

rl_value_t real_unwrap( value_t val )
{
  return (rl_value_t)val;
}

rl_value_t imm_unwrap( value_t val )
{
  return (rl_value_t)(val&IMM_MASK);
}

cv_unwrap( 8 );
cv_unwrap( 16 );
cv_unwrap( 32 );
cv_unwrap( 64 );

// wrap implemenations
#define cv_wrap( size )						\
  value_t cv##size##_wrap( type_t *type, rl_value_t x )		\
  {								\
    uint##size##_t buffer = x.as_uint##size;			\
    object_t *wrapper     = new_obj( type, size>>3, &buffer );	\
    return (value_t)wrapper | type->dtype->tag;			\
  }

value_t fixnum_wrap( type_t *type, rl_value_t x )
{
  return (x.as_fixnum&PTR_MASK)|type->dtype->tag;
}

value_t real_wrap( type_t *type, rl_value_t x )
{
  (void)type;
  return x.as_data;
}

value_t nul_wrap( type_t *type, rl_value_t x )
{
  (void)x;
  return type->dtype->singleton;
}

value_t imm_wrap( type_t *type, rl_value_t x )
{
  return x.as_data|type->dtype->tag|((value_t)type->dtype->repr<<32);
}

cv_wrap( 8 );
cv_wrap( 16 );
cv_wrap( 32 );
cv_wrap( 64 );

// box implementations
int fixnum_unbox( void *dst, value_t val )
{
  *(fixnum_t*)dst = fixnum_unwrap( val ).as_fixnum;
  return sizeof(fixnum_t);
}

int real_unbox( void *dst, value_t val )
{
  *(real_t*)dst = as_real( val );
  return sizeof(real_t);
}

int nul_unbox( void *dst, value_t val )
{
  assert( val ==NUL_VAL );
  *(void**)dst = NULL;
  return 8;
}

#define imm_unbox( size )					\
  int imm##size##_unbox( void *dst, value_t val )		\
  {								\
    *(uint##size##_t*)dst = imm_unwrap(val).as_uint##size;	\
    return size>>3;						\
  }

#define cv_unbox( size )						\
  int cv##size##_unbox( void *dst, value_t val )			\
  {									\
    *(uint##size##_t*)dst = cv##size##_unwrap(val).as_uint##size;	\
    return size>>3;							\
  }

imm_unbox( 8 );
imm_unbox( 16 );
imm_unbox( 32 );
cv_unbox( 8 );
cv_unbox( 16 );
cv_unbox( 32 );
cv_unbox( 64 );
