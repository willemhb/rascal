#ifndef rascal_obj_array_h
#define rascal_obj_array_h

#include <stdarg.h>

#include "obj/object.h"

// a complete API for array types (both internal and external)
#define ARRAY(V)				\
  OBJECT					\
  size_t  length;				\
  size_t  capacity;				\
  V      *data

typedef struct
{
  ARRAY(void);
} array_t;

// function pointer types
typedef size_t     (*pad_t)(size_t oldl, size_t newl, size_t oldc, size_t minc);
typedef rl_value_t (*aref_t)(object_t *obj, size_t n);
typedef rl_value_t (*aset_t)(object_t *array, size_t n, rl_value_t x);
typedef size_t     (*writeto_t)(object_t *dst, void *src, size_t n);
typedef size_t     (*readfrom_t)(object_t *src, void *dst, size_t n);
typedef size_t     (*appendto_t)(object_t *array, size_t n, va_list va);

typedef struct
{
  OBJECT

  arity_t     elsize;
  Ctype_t     Ctype;

  size_t      minc;

  pad_t       pad;
  aref_t      aref;
  aset_t      aset;
  appendto_t  appendto;
} arr_impl_t;

typedef enum
  {
    UNDERFLOW   =-1,
    OKAY        = 0,
    OVERFLOW    = 1,
    NOTSUPPORTED= 2,
    NONARRAY    = 3,
    WRONGTYPE   = 4,
  } arr_err_t;

// forward declarations
bool         val_is_arr( value_t val );
bool         obj_is_arr( object_t *obj );

arr_impl_t  *obj_arr_impl( object_t *obj );
arr_impl_t  *type_arr_impl( type_t *type );

signature_t *obj_arr_types( object_t *obj );
signature_t *type_arr_types( type_t *type );

#define    rl_is_arr( x ) _Generic((x), value_t:val_is_arr, object_t*:obj_is_arr)((x))

#define    rl_arr_impl( x ) \
  _Generic((x),\
	   object_t*:obj_arr_impl,          \
	   type_t*:type_arr_impl )((x))

#define    rl_arr_types( x ) \
  _Generic( (x),\
	    object_t*:obj_arr_types,\
	    type_t*:type_arr_types )((x))

#define    rl_arr_type( x )    (rl_arr_types(x)->data[0])

#define    rl_arr_elsize( x )   ( rl_arr_impl( x )->elsize )
#define    rl_arr_Ctype( x )    ( rl_arr_impl(x)->Ctype )
#define    rl_arr_minc( x )     ( rl_arr_impl(x)->minc )
#define    rl_arr_pad( x )      ( rl_arr_impl(x)->pad )
#define    rl_arr_aref( x )     ( rl_arr_impl(x)->aref)
#define    rl_arr_aset( x )     ( rl_arr_impl(x)->aset )
#define    rl_arr_appendto( x ) ( rl_arr_impl(x)->appendto )

#define    rl_arr_public( x )   ( rl_arr_aref( x ) != NULL )
#define    rl_arr_mutable( x )  ( rl_arr_aset( x ) != NULL )
#define    rl_arr_dynamic( x )  ( rl_arr_pad( x ) != NULL )
#define    rl_arr_encoded( x )  ( is_encoded( rl_arr_Ctype( x ) ) )

size_t     pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc );

object_t  *new_dynamic_array( type_t *type, size_t n );
object_t  *new_immutable_array( type_t *type, size_t n );
object_t  *new_fixed_array( type_t *type, size_t n );

void       init_dynamic_array( object_t *self, type_t *type, size_t n, void *data );
void       init_fixed_array( object_t *self, type_t *type, size_t n, void *data );

void       resize_array( object_t *obj, size_t newl );

void       trace_values( object_t *obj );
void       trace_objects( object_t *obj );
void       free_array( object_t *obj );

// these methods perform minimal checks. Suitable for internal use where safety is
// already assured
rl_value_t rl_aref( object_t *arr, long i );
rl_value_t rl_aset( object_t *arr, long i, rl_value_t x );
rl_value_t rl_pop(  object_t *arr );
rl_value_t rl_popn( object_t *arr, size_t n );
size_t     rl_push( object_t *obj, rl_value_t x );
size_t     rl_pushn( object_t *obj, size_t n );
size_t     rl_insert( object_t *obj, long i, rl_value_t x );
size_t     rl_insertn( object_t *obj, long i, size_t n, void *buf );
rl_value_t rl_popat( object_t *obj, long i );
rl_value_t rl_popnat( object_t *obj, long i, size_t n );
size_t     rl_writeto( object_t *dst, void *src, size_t n );
size_t     rl_readfrom( object_t *src, void *dst, size_t n );
size_t     rl_appendto( object_t *obj, size_t n, ...);

// these methods check bounds and ensure the operation is supported on the given array
// for the sake of consistency, methods that don't really need a safe counterpart are
// still implemented
arr_err_t  rl_aref_s( object_t *obj, long i, rl_value_t *buf );
arr_err_t  rl_aset_s( object_t *obj, long i, rl_value_t x, rl_value_t *buf );
arr_err_t  rl_pop_s( object_t *obj, rl_value_t *buf );
arr_err_t  rl_popn_s( object_t *obj, size_t n, rl_value_t *buf );
arr_err_t  rl_push_s( object_t *obj, rl_value_t x, size_t *buf );
arr_err_t  rl_pushn_s( object_t *obj, size_t n, size_t *buf );
arr_err_t  rl_insert_s( object_t *obj, long i, rl_value_t x, size_t *buf );
arr_err_t  rl_insertn_s( object_t *obj, long i, size_t n, void *src, size_t *buf );
arr_err_t  rl_popat_s( object_t *obj, long i, rl_value_t *buf );
arr_err_t  rl_popnat_s( object_t *obj, long i, size_t n, rl_value_t *buf );
arr_err_t  rl_writeto_s( object_t *dst, void *src, size_t n, size_t *buf );
arr_err_t  rl_readfrom_s( object_t *src, void *dst, size_t n, size_t *buf );
arr_err_t  rl_appendto_s( object_t *src, size_t n, size_t *buf, ...);

// these methods perform the same checks as the _ss methods, but also enforce type safety
arr_err_t  rl_aref_ss( object_t *obj, long i, rl_value_t *buf );
arr_err_t  rl_aset_ss( object_t *obj, long i, value_t x, rl_value_t *buf );
arr_err_t  rl_pop_ss( object_t *obj, rl_value_t *buf );
arr_err_t  rl_popn_ss( object_t *obj, size_t n, rl_value_t *buf );
arr_err_t  rl_push_ss( object_t *obj, value_t x, size_t *buf );
arr_err_t  rl_pushn_ss( object_t *obj, size_t n, size_t *buf );
arr_err_t  rl_insert_ss( object_t *obj, long i, value_t x, size_t *buf );
arr_err_t  rl_insertn_ss( object_t *obj, long i, size_t n, void *src, size_t *buf );
arr_err_t  rl_popat_ss( object_t *obj, long i, rl_value_t *buf );
arr_err_t  rl_popnat_ss( object_t *obj, long i, size_t n, rl_value_t *buf );
arr_err_t  rl_writeto_ss( object_t *dst, void *src, size_t n, size_t *buf );
arr_err_t  rl_readfrom_ss( object_t *src, void *dst, size_t n, size_t *buf );
arr_err_t  rl_appendto_ss( object_t *src, size_t n, size_t *buf, ...);

// just defining all the aref/aset methods we might want here
rl_value_t rl_sint8_aref( object_t *obj, size_t n );
rl_value_t rl_sint8_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_sint8_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_uint8_aref( object_t *obj, size_t n );
rl_value_t rl_uint8_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_uint8_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_ascii_aref( object_t *obj, size_t n );
rl_value_t rl_ascii_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_ascii_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_utf8_aref( object_t *obj, size_t n );
rl_value_t rl_utf8_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_utf8_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_latin1_aref( object_t *obj, size_t n );
rl_value_t rl_latin1_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_latin1_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_sint16_aref( object_t *obj, size_t n );
rl_value_t rl_sint16_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_sint16_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_uint16_aref( object_t *obj, size_t n );
rl_value_t rl_uint16_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_uint16_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_utf16_aref( object_t *obj, size_t n );
rl_value_t rl_utf16_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_utf16_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_sint32_aref( object_t *obj, size_t n );
rl_value_t rl_sint32_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_sint32_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_uint32_aref( object_t *obj, size_t n );
rl_value_t rl_uint32_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_uint32_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_real32_aref( object_t *obj, size_t n );
rl_value_t rl_real32_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_real32_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_utf32_aref( object_t *obj, size_t n );
rl_value_t rl_utf32_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_utf32_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_sint64_aref( object_t *obj, size_t n );
rl_value_t rl_sint64_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_sint64_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_uint64_aref( object_t *obj, size_t n );
rl_value_t rl_uint64_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_uint64_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_real64_aref( object_t *obj, size_t n );
rl_value_t rl_real64_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_real64_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_string_aref( object_t *obj, size_t n );
rl_value_t rl_string_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_string_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_object_aref( object_t *obj, size_t n );
rl_value_t rl_object_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_object_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_pointer_aref( object_t *obj, size_t n );
rl_value_t rl_pointer_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_pointer_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_value_aref( object_t *obj, size_t n );
rl_value_t rl_value_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_value_appendto( object_t *obj, size_t n, va_list va );

rl_value_t rl_fixnum_aref( object_t *obj, size_t n );
rl_value_t rl_fixnum_aset( object_t *obj, size_t n, rl_value_t x );
size_t     rl_fixnum_appendto( object_t *obj, size_t n, va_list va );

// globals
extern layout8_t ArrayLayout;
extern type_t ArrayImplType;

// initialization
void rl_obj_array_init( void );

// convenience
#define as_arr( x )   ( (array_t*)as_obj( x ) )
#define arr_len( x )  ( as_arr( x )->length )
#define arr_cap( x )  ( as_arr( x )->capacity )
#define arr_data( x ) ( as_arr( x )->data )

#endif
