#ifndef rascal_obj_table_h
#define rascal_obj_table_h

#include "obj/object.h"

// general operations on tables & generic table type
#define TABLE					\
  OBJECT					\
  size_t     length;				\
  size_t     capacity;				\
  object_t **data

#define ENTRY(K, key)				\
  OBJECT                                        \
  hash_t     hash;                              \
  K          key

typedef size_t     (*padfn_t)( size_t oldl, size_t newl, size_t oldc, size_t minc );
typedef size_t     (*sizeof_t)( rl_value_t x );
typedef hash_t     (*hashfn_t)( rl_value_t k );
typedef int        (*cmpfn_t)(  rl_value_t x, rl_value_t y );
typedef rl_value_t (*getter_t)( object_t *object );
typedef rl_value_t (*setter_t)( object_t *object, rl_value_t value );

typedef struct
{
  TABLE;
} table_t;

typedef struct
{
  ENTRY(rl_value_t, key);
} entry_t;

typedef struct
{
  rl_value_t key;
  hash_t     hash;
} set_init_t;

typedef struct
{
  rl_value_t key;
  hash_t     hash;
  rl_value_t val;
} map_init_t;

typedef struct
{
  OBJECT

  size_t    minc;
  padfn_t   padlength;
  sizeof_t  keysize;
  hashfn_t  hashkey;
  cmpfn_t   cmpkeys;
  getter_t  getkey;
  getter_t  getvalue;
  setter_t  setvalue;
} tb_impl_t;

static inline types_t *table_signature( object_t *obj )
{
  return (types_t*)obtype(obj)->dtype->types;
}

static inline tb_impl_t *table_impl( object_t *obj )
{
  return (tb_impl_t*)obtype(obj)->dtype->impl;
}

static inline type_t *table_key_type( object_t *obj )
{
  return table_signature( obj )->data[0];
}

static inline type_t *table_value_type( object_t *obj )
{
  return table_signature( obj )->data[1];
}

static inline type_t *table_entry_type( object_t *obj )
{
  return table_signature( obj )->data[2];
}

static inline hash_t table_hash_key( object_t *obj, rl_value_t key )
{
  return table_impl( obj )->hashkey( key );
}

static inline rl_value_t table_get_value( object_t *obj, object_t *entry )
{
  return table_impl( obj )->getvalue( entry );
}

static inline rl_value_t table_get_key( object_t *obj, object_t *entry )
{
  return table_impl( obj )->getkey( entry );
}

static inline rl_value_t table_set_value( object_t *obj, object_t *entry, rl_value_t val )
{
  return table_impl( obj )->setvalue( entry, val );
}

static inline size_t table_keysize( object_t *obj, rl_value_t key )
{
  return table_impl( obj )->keysize( key );
}

static inline size_t table_pad_length( object_t *obj, size_t n )
{
  return table_impl( obj )->padlength( ((table_t*)obj)->length,
				       n,
				       ((table_t*)obj)->capacity,
				       table_impl( obj )->minc );
}

// globals
extern type_t TableImplType;
extern layout8_t TableLayout;

// forward declarations
size_t     pad_table_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
void       trace_table( object_t *object );
void       free_table( object_t *object );
void       resize_table( object_t *object, size_t newl );
object_t **rehash_table( object_t *object, object_t **news, size_t newc );

bool table_assc( object_t *obj, rl_value_t *buf, rl_value_t key );
bool table_intern( object_t *obj, rl_value_t *buf, rl_value_t key );
bool table_popi( object_t *obj, rl_value_t *buf, rl_value_t key );

bool map_get(  object_t *obj, rl_value_t *buf, rl_value_t key );
bool map_add(  object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val );
bool map_put(  object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val );
bool map_popk( object_t *obj, rl_value_t *buf, rl_value_t key );
bool map_popi( object_t *obj, rl_value_t *buf, rl_value_t key );

bool set_has( object_t *obj, rl_value_t key );
bool set_add( object_t *obj, rl_value_t key );
bool set_rmv( object_t *obj, rl_value_t key );

void init_map( object_t *obj, type_t *type, size_t n, void *data );
void init_set( object_t *obj, type_t *type, size_t n, void *data );

// silly initialization stuff
void rl_obj_table_init( void );

#endif
