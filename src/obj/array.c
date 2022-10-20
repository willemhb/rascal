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


arr_impl_t *obj_arr_impl( object_t *obj )
{
  if (obtype(obj) == &DataType)
    return type_arr_impl( (type_t*)obj );
  
  return type_arr_impl( obtype(obj) );
}

arr_impl_t *arr_arr_impl( array_t *arr )
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

types_t *arr_arr_types( array_t *arr )
{
  return type_arr_types( obtype(arr) );
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
  array_t *arr = (array_t*)self;

  arr->length   = 0;
  arr->capacity = rl_arr_pad( type )( 0, n, 0, rl_arr_minc( type ) );
  arr->data     = alloc( arr->capacity * rl_arr_elsize( type ) );

  if ( data )
    memcpy( arr->data, data, arr->length * rl_arr_elsize( type ) );
}

void init_fixed_array( object_t *self, type_t *type, size_t n, void *data )
{
  array_t *arr  = (array_t*)self;
  
  arr->length   = n;
  arr->data     = (void*)(arr+1); // allocated immediately after the array

  if ( data )
    memcpy( arr->data, data, arr->length * rl_arr_elsize( type ) );
}

void resize_array( object_t *obj, size_t newl )
{
  array_t *arr = (array_t*)obj;
  size_t oldc  = arr->capacity, oldl = arr->length;
  pad_t pad    = rl_arr_pad( arr );

  assert( pad );
  
  size_t newc = pad( oldl, newl, oldc, rl_arr_minc( arr ) );
  arr->length = newl;
  
  if ( newc == oldc )
    return;

  size_t elsize = rl_arr_elsize( arr );
  
  arr->data = resize( arr->data, oldc * elsize, newc * elsize );
}

// trace method for arrays of tagged values
void trace_values( object_t *obj )
{
  array_t *arr = (array_t*)obj;

  mark_vals( arr->data, arr->length );
}

// trace method for arrays of untagged objects
void trace_objects( object_t *obj )
{
  array_t *arr = (array_t*)obj;

  mark_objs( arr->data, arr->length );
}

void free_array( object_t *obj )
  {
    array_t *arr = (array_t*)obj;				
    dealloc( arr->data, arr->capacity * rl_arr_elsize( arr ) );
  }

rl_value_t rl_aref( array_t *arr, long i )
{
  if (i < 0)
    i += arr->length;

  return rl_arr_aref( arr )( arr, i );
}

rl_value_t rl_aset( array_t *arr, long i, rl_value_t x )
{
  if ( i < 0 )
    i += arr->length;

  return rl_arr_aset( arr )( arr, i, x );
}

rl_value_t rl_pop( array_t *arr )
{
  rl_value_t out = rl_arr_aref( arr )( arr, arr->length-1 );
  resize_array( (object_t*)arr, arr->length-1);
  return out;
}

rl_value_t rl_popn( array_t *arr, size_t n )
{
  rl_value_t out = rl_arr_aref( arr )( arr, arr->length-1 );
  resize_array( (object_t*)arr, arr->length-n );
  return out;
}

size_t rl_push( array_t *arr, rl_value_t val )
{
  size_t out = arr->length;
  resize_array( &arr->obj, arr->length+1 );
  rl_aset( arr, out, val );
  return out;
}

size_t rl_pushn( array_t *arr, size_t n )
{
  size_t out = arr->length;
  resize_array( &arr->obj, arr->length+n );
  return out;
}

size_t rl_insert( array_t *arr, long i, rl_value_t x )
{
    if( i == -1 )
      return rl_push( arr, x );
    if ( i < 0 )
      i += arr->length;
    resize_array( &arr->obj, arr->length+1 );
    size_t elsize = rl_arr_elsize( arr );
    memmove( arr->data+(i+1)*elsize,
	     arr->data+i*elsize,
	     (arr->length-i-1)*elsize );
    rl_arr_aset( arr )( arr, i, x );
    return i;
  }

size_t rl_insertn( array_t *arr, long i, size_t n, void *buf )
{
  size_t elsize = rl_arr_elsize( arr );
  
  if ( i == -1 )
      i = rl_pushn( arr, n );

  else
    {
      if ( i < 0 )
	i += arr->length;

      resize_array( &arr->obj, arr->length+n );
      memmove( arr->data+(i+n)*elsize,
	       arr->data+i*elsize,
	       (arr->length-i-n)*elsize );
    }

  if (buf)
    memcpy( arr->data+i*elsize, buf, n*elsize );

  return i;
}

rl_value_t rl_popat( array_t *arr, long i )
{   
    if ( i == -1 )
      return rl_pop( arr );

    if ( i < 0 )
      i += arr->length;

    rl_value_t out = rl_arr_aref( arr )( arr, i );
    size_t elsize = rl_arr_elsize( arr );
    memmove( arr->data+i*elsize,
	     arr->data+(i+1)*elsize,
	     ( arr->length-i-1 )*elsize );

    resize_array( &arr->obj, arr->length-1 );
    return out;
  }

rl_value_t rl_popnat( array_t *arr, long i, size_t n )
  {
    if ( i == -1 )			     
      rl_popn( arr, n );
    if ( i < 0 )
      i += arr->length;
    rl_value_t out = rl_arr_aref( arr )( arr, i );
    size_t elsize = rl_arr_elsize( arr );
    memmove( arr->data+i*elsize,
	     arr->data+(i+n)*elsize,
	    (arr->length-i-n )*elsize );
    resize_array( &arr->obj, arr->length-n );
    return out;
  }

size_t rl_writeto( array_t *dst, void *src, size_t n )
{
  resize_array( &dst->obj, dst->length+n );
  memcpy( dst->data, src, n * rl_arr_elsize( dst ) );
  return dst->length;
}

size_t rl_readfrom( array_t *src, void *dst, size_t n )
{
  n = min( n, src->length );
  memcpy( src->data, dst, n * rl_arr_elsize( src ) );
  return n;
}

size_t rl_appendto( array_t *arr, size_t n, ...)
{
  va_list va;
  va_start(va, n);
  size_t out = rl_arr_appendto( arr )( arr, n, va );
  va_end(va);
  return out;
}

arr_err_t rl_aref_s( array_t *arr, long i, rl_value_t *buf )
{
  if (i < 0)
    i += arr->length;

  if (i < 0)
    return UNDERFLOW;

  if ((size_t)i > arr->length)
    return OVERFLOW;

  if (rl_arr_aref( arr ) == NULL)
    return NOTSUPPORTED;

  if (buf)
    *buf = rl_arr_aref( arr )( arr,  i );

  return OKAY;
}

arr_err_t rl_aref_ss( object_t *obj, long i, rl_value_t *buf )
{
  if (obj == NULL || !rl_is_arr(obj))
    return NONARRAY;

  return rl_aref_s( (array_t*)obj, i, buf );
}

arr_err_t rl_aset_s( array_t *arr, long i, rl_value_t x, rl_value_t *buf )
{
  if ( i < 0 )
    i += arr->length;

  if ( i < 0 )
    return UNDERFLOW;

  if ( (size_t)i > arr->length )
    return OVERFLOW;

  if ( !rl_arr_mutable( arr ) || rl_arr_aset( arr ) == NULL )
    return NOTSUPPORTED;

  rl_value_t out = rl_arr_aset( arr )( arr, i, x );

  if (buf)
    *buf = out;

  return OKAY;
}

extern rl_value_t rl_unbox( type_t *type, value_t x );

arr_err_t rl_aset_ss( object_t *obj, long i, value_t x, rl_value_t *buf )
{
  if ( !rl_is_arr(obj) )
    return NONARRAY;

  type_t *eltype = rl_arr_type( obj );

  if (!eltype->isa( eltype, x) )
    return WRONGTYPE;
  
  rl_value_t xv = rl_unbox( eltype, x );

  return rl_aset_s( (array_t*)obj, i, xv, buf );
}

arr_err_t rl_pop_s( array_t *arr, rl_value_t *buf )
{
  if (arr->length == 0)
    return UNDERFLOW;

  rl_value_t out = rl_pop( arr );

  if (buf)
    *buf = out;

  return OKAY;
}

arr_err_t rl_pop_ss( object_t *obj, rl_value_t *buf )
{
  if ( !rl_is_arr(obj) )
    return NONARRAY;

  return rl_pop_s( (array_t*)obj, buf );
}

arr_err_t rl_popn_s( array_t *arr, size_t n, rl_value_t *buf )
{
  if (arr->length < n)
    return UNDERFLOW;

  rl_value_t out = rl_popn( arr, n );

  if ( buf )
    *buf = out;

  return OKAY;
}

arr_err_t rl_popn_ss( object_t *obj, size_t n, rl_value_t *buf )
{
  if ( !rl_is_arr(obj) )
    return NONARRAY;

  return rl_popn_s( (array_t*)obj, n, buf );
}

arr_err_t rl_push_s( array_t *arr, rl_value_t x, size_t *buf )
{
  size_t out = rl_push( arr, x );
  if (buf)
    *buf = out;
  return OKAY;
}

arr_err_t rl_push_ss( object_t *obj, value_t x, size_t *buf )
{
  if ( !rl_is_arr(obj) )
    return NONARRAY;

  type_t *eltype = rl_arr_type( obj );

  if ( !eltype->isa( eltype, x ) )
    return WRONGTYPE;

  return rl_push_s( (array_t*)obj, rl_unbox(eltype, x), buf );
}

arr_err_t rl_pushn_s( array_t *arr, size_t n, size_t *buf )
{
  size_t out = rl_pushn( arr, n );
  if (buf)
    *buf = out;
  return OKAY;
}

arr_err_t rl_pushn_ss( object_t *obj, size_t n, size_t *buf )
{
  if ( !rl_is_arr(obj) )
    return NONARRAY;

  return rl_pushn_s( (array_t*)obj, n, buf );
}

arr_err_t rl_insert_s( array_t *arr, )

#define ARRAY_LAYOUT( T, V )						\
  { .base_size = sizeof(T##_t), .n_fields = 4 },			\
  {									\
    { sizeof(size_t), NTUINT, false, offsetof(T##_t, length) },		\
    { sizeof(size_t), NTUINT, false, offsetof(T##_t, capacity) },	\
    { sizeof(V*), NTPTR, false, offsetof(T##_t, data) }		        \
  }
