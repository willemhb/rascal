#include <string.h>

#include "obj/array.h"
#include "vm/memory.h"
#include "utils/numutils.h"


// general array utilities
bool val_is_arr( value_t val )
{
  return is_obj(val) && obj_is_arr( as_obj(val) );
}

bool obj_is_arr( object_t *obj )
{
  if (obj == NULL)
    return false;
  
  type_t *type = obtype( obj );

  return type->dtype->impl
    && obtype( type->dtype->impl ) == &ArrayImplType;
}

size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity32_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;
  if (newc < minc)
    newc = minc;
  return newc;
}

size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;
}

size_t pad_vector_size( size_t oldl, size_t newl, size_t oldc, size_t minc )
{
  (void)oldl;
  (void)oldc;
  (void)minc;

  return newl;
}

size_t pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc )
{
  return pad_stack_size( oldl, newl, oldc, minc ) + 1;
}

size_t pad_string_size( size_t oldl, size_t newl, size_t oldc, size_t minc )
{
  (void)oldl;
  (void)oldc;
  (void)minc;

  return newl + 1;
}


arr_impl_t *obj_arr_impl( object_t *arr )
{
  return type_arr_impl( obtype(arr) );
}

arr_impl_t *type_arr_impl( type_t *type )
{
  return (arr_impl_t*)type->dtype->impl;
}

types_t *obj_arr_types( object_t *obj )
{
  return type_arr_types( obtype(obj) );
}

types_t *type_arr_types( type_t *type )
{
  return ((types_t*)type->dtype->types);
}

object_t *new_dynamic_array( type_t *type, size_t n )
{
  (void)n;
  return allocob( type_base_size( type ) );
}

object_t *new_immutable_array ( type_t *type, size_t n )
{
  if (n == 0)
    return as_obj( type->dtype->singleton );

  return new_fixed_array( type, n );
}

object_t *new_fixed_array( type_t *type, size_t n )
{
  if ( rl_arr_pad( type ) )
    n = rl_arr_pad( type )( 0, n, 0, rl_arr_minc( type ) );

  array_t *out = (array_t*)allocob( type_base_size( type ) + n * rl_arr_elsize( type ) );

  // initialize here so it doesn't have to be computed again
  out->capacity = n;

  return &out->obj;
}

void init_dynamic_array( object_t *self, type_t *type, size_t n, void *data )
{
  arr_len( self )  = 0;
  arr_cap( self )  = rl_arr_pad( type )( 0, n, 0, rl_arr_minc( type ) );
  arr_data( self ) = alloc( arr_cap( self ) * rl_arr_elsize( type ) );

  if ( data )
    memcpy( arr_data( self ), data, arr_len( self ) * rl_arr_elsize( type ) );
}

void init_fixed_array( object_t *self, type_t *type, size_t n, void *data )
{
  arr_len( self )  = n;
  arr_data( self ) = (void*)(as_arr( self )+1); // allocated immediately after the array

  if ( data )
    memcpy( arr_data( self ), data, arr_len( self ) * rl_arr_elsize( type ) );
}

void resize_array( object_t *obj, size_t newl )
{
  size_t oldc    = arr_cap( obj ), oldl = arr_len( obj );
  pad_t pad      = rl_arr_pad( obj );

  assert( pad );
  
  size_t newc    = pad( oldl, newl, oldc, rl_arr_minc( obj ) );
  arr_len( obj ) = newl;
  
  if ( newc == oldc )
    return;

  size_t elsize = rl_arr_elsize( obj );
  
  arr_data( obj ) = resize( arr_data( obj ), oldc * elsize, newc * elsize );
}

// trace method for arrays of tagged values
void trace_values( object_t *obj )
{
  mark_vals( arr_data( obj ), arr_len( obj ) );
}

// trace method for arrays of untagged objects
void trace_objects( object_t *obj )
{
  mark_objs( arr_data( obj ), arr_len( obj ) );
}

void free_array( object_t *obj )
  {
    dealloc( arr_data( obj ), arr_cap( obj ) * rl_arr_elsize( obj ) );
  }


#define PRELUDE_INDEX					\
  if ( i < 0 )						\
    i += arr_len( obj )

#define PRELUDE_INDEX_S                                 \
  PRELUDE_INDEX;                                        \
  if ( i < 0 )						\
    return UNDERFLOW;					\
  if ( (size_t)i > arr_len( obj ) )			\
    return OVERFLOW

#define PRELUDE_UNDERFLOW_S( n )			\
  if ( (n) > arr_len( obj ) )				\
    return UNDERFLOW

#define PRELUDE_ISEND( op, args... )		\
  if ( i == -1 )				\
    return op( args )

#define PRELUDE_SUPPORTED( op )			        \
  if ( rl_arr_##op( obj ) == NULL )			\
    return NOTSUPPORTED

#define PRELUDE_ARRAY_S				\
  if ( !rl_is_arr( obj ) )			\
    return NONARRAY

#define PRELUDE_ELTYPE_S					\
  if ( !rl_arr_type( obj )->isa( rl_arr_type( obj ), x ) )	\
    return WRONGTYPE

#define RETURN_S( result )			\
  if (buf)					\
    *buf = result;				\
  return OKAY

// API methods
rl_value_t rl_aref( object_t *obj, long i )
{
  PRELUDE_INDEX;
  return rl_arr_aref( obj )( obj, i );
}

rl_value_t rl_aset( object_t *obj, long i, rl_value_t x )
{
  PRELUDE_INDEX;
  return rl_arr_aset( obj )( obj, i, x );
}

rl_value_t rl_pop( object_t *obj )
{
  rl_value_t out = rl_arr_aref( obj )( obj, arr_len( obj ) - 1 );
  resize_array( obj, arr_len( obj ) - 1 );
  return out;
}

rl_value_t rl_popn( object_t *obj, size_t n )
{
  rl_value_t out = rl_arr_aref( obj )( obj, arr_len( obj ) - 1 );
  resize_array( obj, arr_len( obj ) - n );
  return out;
}

size_t rl_push( object_t *obj, rl_value_t val )
{
  size_t out = arr_len( obj );
  resize_array( obj, arr_len( obj ) + 1 );
  rl_aset( obj, out, val );
  return out;
}

size_t rl_pushn( object_t *obj, size_t n )
{
  size_t out = arr_len( obj );
  resize_array( obj, arr_len( obj )+n );
  return out;
}

size_t rl_insert( object_t *obj, long i, rl_value_t x )
{
  PRELUDE_ISEND( rl_push, obj, x );
  PRELUDE_INDEX;

  resize_array( obj, arr_len( obj )+1 );
  size_t elsize = rl_arr_elsize( obj );

  memmove( arr_data( obj )+(i+1)*elsize,
	   arr_data( obj )+i*elsize,
	   (arr_len( obj )-i-1)*elsize );
  rl_arr_aset( obj )( obj, i, x );
  return i;
}

size_t rl_insertn( object_t *obj, long i, size_t n, void *buf )
{
  size_t elsize = rl_arr_elsize( obj );
  
  if ( i == -1 )
      i = rl_pushn( obj, n );

  else
    {
      if ( i < 0 )
	i += arr_len( obj );

      resize_array( obj, arr_len( obj )+n );
      memmove( arr_data( obj )+(i+n)*elsize,
	       arr_data( obj )+i*elsize,
	       (arr_len( obj )-i-n)*elsize );
    }

  if (buf)
    memcpy( arr_data( obj )+i*elsize, buf, n*elsize );

  return i;
}

rl_value_t rl_popat( object_t *obj, long i )
{   
    if ( i == -1 )
      return rl_pop( obj );

    if ( i < 0 )
      i += arr_len( obj );

    rl_value_t out = rl_arr_aref( obj )( obj, i );
    size_t elsize = rl_arr_elsize( obj );
    memmove( arr_data( obj )+i*elsize,
	     arr_data( obj )+(i+1)*elsize,
	     ( arr_len( obj )-i-1 )*elsize );

    resize_array( obj, arr_len( obj )-1 );
    return out;
  }

rl_value_t rl_popnat( object_t *obj, long i, size_t n )
  {
    if ( i == -1 )			     
      rl_popn( obj, n );
    if ( i < 0 )
      i += arr_len( obj );
    rl_value_t out = rl_arr_aref( obj )( obj, i );
    size_t elsize = rl_arr_elsize( obj );
    memmove( arr_data( obj )+i*elsize,
	     arr_data( obj)+(i+n)*elsize,
	    (arr_len( obj )-i-n )*elsize );
    resize_array( obj, arr_len( obj )-n );
    return out;
  }

size_t rl_writeto( object_t *dst, void *src, size_t n )
{
  resize_array( dst, arr_len( dst )+n );
  memcpy( arr_data( dst ), src, n * rl_arr_elsize( dst ) );
  return arr_len( dst );
}

size_t rl_readfrom( object_t *src, void *dst, size_t n )
{
  n = min( n, arr_len( src ) );
  memcpy( arr_data( src ), dst, n * rl_arr_elsize( src ) );
  return n;
}

size_t rl_appendto( object_t *obj, size_t n, ...)
{
  va_list va;
  va_start(va, n);
  size_t out = rl_arr_appendto( obj )( obj, n, va );
  va_end(va);
  return out;
}

arr_err_t rl_aref_s( object_t *obj, long i, rl_value_t *buf )
{
  PRELUDE_SUPPORTED( aref );
  PRELUDE_INDEX_S;
  RETURN_S( rl_arr_aref( obj )( obj, i ) );
}

arr_err_t rl_aref_ss( object_t *obj, long i, rl_value_t *buf )
{
  PRELUDE_ARRAY_S;
  return rl_aref_s( obj, i, buf );
}

arr_err_t rl_aset_s( object_t *obj, long i, rl_value_t x, rl_value_t *buf )
{
  PRELUDE_SUPPORTED( aset );
  PRELUDE_INDEX_S;

  rl_value_t out = rl_arr_aset( obj )( obj, i, x );

  RETURN_S( out );
}

extern rl_value_t rl_unbox( type_t *type, value_t x );

arr_err_t rl_aset_ss( object_t *obj, long i, value_t x, rl_value_t *buf )
{
  PRELUDE_ARRAY_S;
  PRELUDE_ELTYPE_S;
  rl_value_t xv = rl_unbox( rl_arr_type( obj ), x );

  return rl_aset_s( obj, i, xv, buf );
}

arr_err_t rl_pop_s( object_t *obj, rl_value_t *buf )
{
  PRELUDE_SUPPORTED( pad );
  PRELUDE_SUPPORTED( aset );
  PRELUDE_UNDERFLOW_S( 1 );

  rl_value_t out = rl_pop( obj );

  RETURN_S( out );
}

arr_err_t rl_pop_ss( object_t *obj, rl_value_t *buf )
{
  PRELUDE_ARRAY_S;
  return rl_pop_s( obj, buf );
}

arr_err_t rl_popn_s( object_t *obj, size_t n, rl_value_t *buf )
{
  PRELUDE_SUPPORTED( pad );
  PRELUDE_SUPPORTED( aset );
  PRELUDE_UNDERFLOW_S( n );

  rl_value_t out = rl_popn( obj, n );

  RETURN_S( out );
}

arr_err_t rl_popn_ss( object_t *obj, size_t n, rl_value_t *buf )
{
  PRELUDE_ARRAY_S;

  return rl_popn_s( obj, n, buf );
}

arr_err_t rl_push_s( object_t *obj, rl_value_t x, size_t *buf )
{
  PRELUDE_SUPPORTED( pad );
  PRELUDE_SUPPORTED( aset );

  size_t out = rl_push( obj, x );

  RETURN_S( out );
}

arr_err_t rl_push_ss( object_t *obj, value_t x, size_t *buf )
{
  PRELUDE_ARRAY_S;
  PRELUDE_ELTYPE_S;
  
  return rl_push_s( obj, rl_unbox( rl_arr_type( obj ), x), buf );
}

arr_err_t rl_pushn_s( object_t *obj, size_t n, size_t *buf )
{
  PRELUDE_SUPPORTED( pad );
  PRELUDE_SUPPORTED( aset );

  size_t out = rl_pushn( obj, n );

  RETURN_S( out );
}

arr_err_t rl_pushn_ss( object_t *obj, size_t n, size_t *buf )
{
  PRELUDE_ARRAY_S;
  return rl_pushn_s( obj, n, buf );
}

arr_err_t rl_insert_s( object_t *obj, long i, rl_value_t x, size_t *buf )
{
  PRELUDE_SUPPORTED( aset );
  PRELUDE_SUPPORTED( pad );
  PRELUDE_INDEX_S;
  
  size_t out = rl_insert( obj, i, x );

  RETURN_S( out );
}

arr_err_t rl_insert_


// array method implementations
// for most conceivable use cases, new array types should be able to select
// from these methods for its get/set/appendto method
#define DESCRIBE_AREF( type, TYPE )		                    \
  rl_value_t rl_##type##_aref( object_t *obj, size_t n )            \
  {                                                                 \
  return (rl_value_t)(((TYPE*)arr_data( obj ))[n]);	            \
  }

#define DESCRIBE_ASET( type, TYPE )                                    \
  rl_value_t rl_##type##_aset( object_t *obj, size_t n, rl_value_t x ) \
  {                                                                    \
    ( (TYPE*)arr_data( obj ) )[n] = x.as_##type;		       \
    return x;							       \
  }

#define DESCRIBE_SMINT_APPENDTO( type, TYPE )                           \
  size_t rl_##type##_appendto( object_t *obj, size_t n, va_list va )    \
  {                                                                     \
    TYPE buffer[n];						        \
    for (size_t i=0; i<n; i++) buffer[i] = va_arg(va, int);             \
    size_t off = arr_len( obj )-1;				        \
    resize_array( obj, arr_len( obj ) + n );			        \
    memcpy( (TYPE*)arr_data( obj ) + off, buffer, n * sizeof( TYPE ) ); \
    return arr_len( obj );						\
  }

#define DESCRIBE_SMFLOAT_APPENDTO( type, TYPE )                         \
  size_t rl_##type##_appendto( object_t *obj, size_t n, va_list va )    \
  {                                                                     \
    TYPE buffer[n];						        \
    for (size_t i=0; i<n; i++) buffer[i] = va_arg(va, double);          \
    size_t off = arr_len( obj )-1;				        \
    resize_array( obj, arr_len( obj ) + n );			        \
    memcpy( (TYPE*)arr_data( obj ) + off, buffer, n * sizeof( TYPE ) ); \
    return arr_len( obj );						\
  }


#define DESCRIBE_APPENDTO( type, TYPE )                                 \
  size_t rl_##type##_appendto( object_t *obj, size_t n, va_list va )    \
  {                                                                     \
    TYPE buffer[n];						        \
    for (size_t i=0; i<n; i++) buffer[i] = va_arg(va, TYPE);            \
    size_t off = arr_len( obj )-1;				        \
    resize_array( obj, arr_len( obj ) + n );			        \
    memcpy( (TYPE*)arr_data( obj ) + off, buffer, n * sizeof( TYPE ) ); \
    return arr_len( obj );						\
  }

DESCRIBE_AREF(sint8, int8_t);
DESCRIBE_ASET(sint8, int8_t);
DESCRIBE_SMINT_APPENDTO( sint8, int8_t );

DESCRIBE_AREF(uint8, uint8_t);
DESCRIBE_ASET(uint8, uint8_t);
DESCRIBE_SMINT_APPENDTO( uint8, uint8_t );

DESCRIBE_AREF(utf8, utf8_t);
DESCRIBE_ASET(utf8, utf8_t);
DESCRIBE_SMINT_APPENDTO( utf8, utf8_t );

DESCRIBE_AREF(ascii, ascii_t);
DESCRIBE_ASET(ascii, ascii_t);
DESCRIBE_SMINT_APPENDTO( ascii, ascii_t );

DESCRIBE_AREF(latin1, latin1_t);
DESCRIBE_ASET(latin1, latin1_t);
DESCRIBE_SMINT_APPENDTO( latin1, latin1_t );

DESCRIBE_AREF(sint16, int16_t);
DESCRIBE_ASET(sint16, int16_t);
DESCRIBE_SMINT_APPENDTO( sint16, int16_t );

DESCRIBE_AREF(uint16, uint16_t);
DESCRIBE_ASET(uint16, uint16_t);
DESCRIBE_SMINT_APPENDTO( uint16, uint16_t );

DESCRIBE_AREF(utf16, utf16_t);
DESCRIBE_ASET(utf16, utf16_t);
DESCRIBE_SMINT_APPENDTO( utf16, utf16_t );

DESCRIBE_AREF( sint32, int32_t );
DESCRIBE_ASET( sint32, int32_t );
DESCRIBE_SMINT_APPENDTO( sint32, int32_t );

DESCRIBE_AREF( uint32, uint32_t );
DESCRIBE_ASET( uint32, uint32_t );
DESCRIBE_APPENDTO( uint32, uint32_t );

DESCRIBE_AREF( utf32, utf32_t );
DESCRIBE_ASET( utf32, utf32_t );
DESCRIBE_APPENDTO( utf32, utf32_t );

DESCRIBE_AREF( real32, real32_t );
DESCRIBE_ASET( real32, real32_t );
DESCRIBE_SMFLOAT_APPENDTO( real32, real32_t );

DESCRIBE_AREF( sint64, int64_t );
DESCRIBE_ASET( sint64, int64_t );
DESCRIBE_APPENDTO( sint64, int64_t );

DESCRIBE_AREF( uint64, uint64_t );
DESCRIBE_ASET( uint64, uint64_t );
DESCRIBE_APPENDTO( uint64, uint64_t );

DESCRIBE_AREF( real64, real64_t );
DESCRIBE_ASET( real64, real64_t );
DESCRIBE_APPENDTO( real64, real64_t );

DESCRIBE_AREF( Cstring, char* );
DESCRIBE_ASET( Cstring, char* );
DESCRIBE_APPENDTO( Cstring, char* );

DESCRIBE_AREF( pointer, pointer_t );
DESCRIBE_ASET( pointer, pointer_t );
DESCRIBE_APPENDTO( pointer, pointer_t );

DESCRIBE_AREF( object, object_t* );
DESCRIBE_ASET( object, object_t* );
DESCRIBE_APPENDTO( object, object_t* );

DESCRIBE_AREF( value, value_t );
DESCRIBE_ASET( value, value_t );
DESCRIBE_APPENDTO( value, value_t );

DESCRIBE_AREF( fixnum, fixnum_t );
DESCRIBE_ASET( fixnum, fixnum_t );
DESCRIBE_APPENDTO( fixnum, fixnum_t );

// globals
layout8_t ArrayLayout =
  {
    { .base_size = sizeof(array_t), .n_fields = 3 },
    {
      { sizeof(size_t), NTUINT, false, offsetof(array_t, length) },
      { sizeof(size_t), NTUINT, false, offsetof(array_t, capacity) },
      { sizeof(void*),  NTPTR,  false, offsetof(array_t, data) }
    }
  };

layout8_t ArrayImplLayout =
  {
    { .base_size = sizeof(arr_impl_t), .n_fields=7 },
    {
      { sizeof(arity_t),    NTUINT, false, offsetof(arr_impl_t, elsize)   },
      { sizeof(Ctype_t),    NTSINT, false, offsetof(arr_impl_t, Ctype)    },
      { sizeof(size_t),     NTUINT, false, offsetof(arr_impl_t, minc)     },
      { sizeof(pad_t),      NTPTR,  false, offsetof(arr_impl_t, pad)      },
      { sizeof(aref_t),     NTPTR,  false, offsetof(arr_impl_t, aref)     },
      { sizeof(aset_t),     NTPTR,  false, offsetof(arr_impl_t, aset)     },
      { sizeof(appendto_t), NTPTR,  false, offsetof(arr_impl_t, appendto) }
    }
  };

dtype_impl_t ArrayImplImpl =
  {
    .value=Ctype_object,
    .repr=OBJ,
    .data_size=sizeof(arr_impl_t*),
    .layout=&ArrayImplLayout.layout,
  };

type_t ArrayImplType =
  {
    { .dtype=&DataType },

    .dtype=&ArrayImplImpl
  };

// initialization
void rl_obj_array_init( void )
{
  init_dtype( &ArrayImplType, "array-impl", NOCONS, 0 );
}
