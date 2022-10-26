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
typedef size_t     (*appendto_t)(object_t *array, size_t n, va_list va);

typedef struct
{
  OBJECT;

  Ctype_t     Ctype;
  arity_t     elsize;

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
size_t     pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc );

object_t  *create_dynamic_array( type_t *type, size_t n );
object_t  *create_immutable_array( type_t *type, size_t n );
object_t  *create_fixed_array( type_t *type, size_t n );

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
extern layout_t ArrayLayout;
extern type_t ArrayImplType;

// initialization
void rl_init_obj_array( void );
void rl_mark_obj_array( void );
void rl_unmark_obj_array( void );
void rl_finalize_obj_array( void );

// convenience
#define is_arr( x )         ( is_array_type( rl_type( x ) ) )
#define as_arr( x )         ( (array_t*)as_obj( x ) )
#define arr_len( x )        ( as_arr( x )->length )
#define arr_cap( x )        ( as_arr( x )->capacity )
#define arr_data( x )       ( as_arr( x )->data )

#define arr_impl( x )       ( type_array_impl( rl_type( x ) ) )
#define arr_elsize( x )     ( array_impl( x )->elsize )
#define arr_Ctype( x )      ( array_impl( x )->Ctype )
#define arr_minc( x )       ( array_impl( x )->minc )
#define arr_pad( x )        ( array_impl( x )->pad )
#define arr_aref( x )       ( array_impl( x )->aref)
#define arr_aset( x )       ( array_impl( x )->aset )
#define arr_appendto( x )   ( array_impl( x )->appendto )

#define arr_is_public( x )  ( array_aref( x ) != NULL )
#define arr_is_mutable( x ) ( array_aset( x ) != NULL )
#define arr_is_dynamic( x ) ( array_pad( x ) != NULL )
#define arr_is_encoded( x ) ( is_encoded( rl_arr_Ctype( x ) ) )

#endif
