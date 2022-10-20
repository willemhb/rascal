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
typedef rl_value_t (*aref_t)(array_t *obj, size_t n);
typedef rl_value_t (*aset_t)(array_t *array, size_t n, rl_value_t x);
typedef size_t     (*writeto_t)(object_t *dst, void *src, size_t n);
typedef size_t     (*readfrom_t)(object_t *src, void *dst, size_t n);
typedef size_t     (*appendto_t)(array_t *array, size_t n, va_list va);

typedef struct
{
  OBJECT

  bool        dynamic;
  bool        mutable;
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
bool        val_is_arr( value_t val );
bool        obj_is_arr( object_t *obj );

arr_impl_t *obj_arr_impl( object_t *obj );
arr_impl_t *arr_arr_impl( array_t *arr );
arr_impl_t *type_arr_impl( type_t *type );

types_t    *obj_arr_types( object_t *obj );
types_t    *arr_arr_types( array_t *arr );
types_t    *type_arr_types( type_t *type );

#define    rl_is_arr( x ) _Generic((x), value_t:val_is_arr, object_t*:obj_is_arr)((x))

#define    rl_arr_impl( x ) \
  _Generic((x),\
	   object_t*:obj_arr_impl,          \
	   type_t*:type_arr_impl,		\
	   array_t*:arr_arr_impl)((x))

#define    rl_arr_types( x ) \
  _Generic((x),\
	   object_t*:obj_arr_types,\
	   type_t*:type_arr_types,\
	   array_t*:arr_arr_types)((x))

#define    rl_arr_type( x )    (rl_arr_types(x)->data[0])

#define    rl_arr_dynamic( x )  (rl_arr_impl(x)->dynamic)
#define    rl_arr_mutable( x )  (rl_arr_impl(x)->mutable)
#define    rl_arr_elsize( x )   (rl_arr_impl(x)->elsize)
#define    rl_arr_Ctype( x )    (rl_arr_impl(x)->Ctype)
#define    rl_arr_minc( x )     (rl_arr_impl(x)->minc)
#define    rl_arr_pad( x )      (rl_arr_impl(x)->pad)
#define    rl_arr_aref( x )     (rl_arr_impl(x)->aref)
#define    rl_arr_aset( x )     (rl_arr_impl(x)->aset)
#define    rl_arr_appendto( x ) (rl_arr_impl(x)->appendto)

size_t     pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t     pad_string_size( size_t oldl, size_t newl, size_t oldc, size_t minc );

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
rl_value_t rl_aref( array_t *arr, long i );
rl_value_t rl_aset( array_t *arr, long i, rl_value_t x );
rl_value_t rl_pop(  array_t *arr );
rl_value_t rl_popn( array_t *arr, size_t n );
size_t     rl_push( array_t *arr, rl_value_t x );
size_t     rl_pushn( array_t *arr, size_t n );
size_t     rl_insert( array_t *arr, long i, rl_value_t x );
size_t     rl_insertn( array_t *arr, long i, size_t n, void *buf );
rl_value_t rl_popat( array_t *arr, long i );
rl_value_t rl_popnat( array_t *arr, long i, size_t n );
size_t     rl_writeto( array_t *dst, void *src, size_t n );
size_t     rl_readfrom( array_t *src, void *dst, size_t n );
size_t     rl_appendto( array_t *arr, size_t n, ...);

// these methods check bounds and ensure the operation is supported on the given array
// for the sake of consistency, methods that don't really need a safe counterpart are
// still implemented
arr_err_t  rl_aref_s( array_t *arr, long i, rl_value_t *buf );
arr_err_t  rl_aset_s( array_t *arr, long i, rl_value_t x, rl_value_t *buf );
arr_err_t  rl_pop_s( array_t *arr, rl_value_t *buf );
arr_err_t  rl_popn_s( array_t *arr, size_t n, rl_value_t *buf );
arr_err_t  rl_push_s( array_t *arr, rl_value_t x, size_t *buf );
arr_err_t  rl_pushn_s( array_t *arr, size_t n, size_t *buf );
arr_err_t  rl_insert_s( array_t *arr, long i, rl_value_t x, size_t *buf );
arr_err_t  rl_insertn_s( array_t *arr, long i, size_t n, void *src, size_t *buf );
arr_err_t  rl_popat_s( array_t *arr, long i, rl_value_t *buf );
arr_err_t  rl_popnat_s( array_t *arr, long i, size_t n, rl_value_t *buf );
arr_err_t  rl_writeto_s( object_t *dst, void *src, size_t n, size_t *buf );
arr_err_t  rl_readfrom_s( array_t *src, void *dst, size_t n, size_t *buf );
arr_err_t  rl_appendto_s( array_t *src, size_t n, ...);

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
arr_err_t  rl_appendto_ss( object_t *src, size_t n, ...);

// globals
extern layout8_t ArrayLayout, StringLayout;
extern type_t ArrayImplType;

// initialization
void rl_obj_array_init( void );

#endif
