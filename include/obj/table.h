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

typedef object_t  *(*intern_t)( object_t *table, rl_value_t key, hash_t hash, size_t j );
typedef size_t     (*pad_t)( size_t oldl, size_t newl, size_t oldc, size_t minc );
typedef size_t     (*sizefn_t)( rl_value_t x );
typedef hash_t     (*hashfn_t)( rl_value_t k );
typedef int        (*compare_t)(  rl_value_t x, rl_value_t y );
typedef rl_value_t (*getter_t)( object_t *object );
typedef rl_value_t (*setter_t)( object_t *object, rl_value_t value );
typedef rl_value_t (*initer_t)( object_t *object, rl_value_t value, bool is_dummy_value );

typedef struct
{
  TABLE;
} table_t;

typedef struct
{
  ENTRY( rl_value_t, key );
} entry_t;

typedef struct
{
  rl_value_t key;
  hash_t     hash;
} set_init_t;

typedef struct map_init_t
{
  rl_value_t key;
  hash_t     hash;
  rl_value_t val;
} map_init_t;

typedef struct
{
  OBJECT

  size_t    minc;
  type_t   *entry_type;
  pad_t     pad;
  sizefn_t  keysize;
  hashfn_t  keyhash;
  compare_t keycmp;
  getter_t  getkey;
  getter_t  getvalue;
  setter_t  setvalue;
  intern_t  intern;
} tb_impl_t;

// globals
extern type_t TableImplType;
extern layout8_t TableLayout;

// forward declarations
size_t     pad_table_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
void       trace_table( object_t *object );
void       free_table( object_t *object );
void       resize_table( object_t *object, size_t newl );
object_t  *new_table( type_t *type, size_t n );
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
bool set_sup( object_t *self, object_t *other );
bool set_sub( object_t *self, object_t *other );
bool set_add( object_t *obj, rl_value_t key );
bool set_rmv( object_t *obj, rl_value_t key );

void init_map( object_t *obj, type_t *type, size_t n, void *data );
void init_set( object_t *obj, type_t *type, size_t n, void *data );

// generally useful getters
size_t     id_keysize( rl_value_t key );
rl_value_t id_getkey( object_t *obj );
rl_value_t id_getval( object_t *obj );
int        id_keycmp( rl_value_t x, rl_value_t y );
hash_t     id_gethash( rl_value_t x );
object_t  *id_intern( object_t *obj, rl_value_t k, hash_t h, size_t j );    // intern the key itself (key should store its own hash)
object_t  *etype_intern( object_t *obj, rl_value_t k, hash_t h, size_t j ); // create a new entry (standard)

// silly initialization stuff
void rl_obj_table_init( void );

// convenience
#define as_table( x )   ( (table_t*)as_obj( x ) )
#define tb_len( x )     ( as_table( x )->length )
#define tb_cap( x )     ( as_table( x )->capacity )
#define tb_data( x )    ( as_table( x )->data )

#define set_len  tb_len
#define set_cap  tb_cap
#define set_data tb_data
#define map_len  tb_len
#define map_cap  tb_cap
#define map_data tb_data

static inline signature_t *tb_sig( object_t *obj )
{
  return ( signature_t* )dtype_signature( obj_type( obj ) );
}

static inline tb_impl_t *tb_impl( object_t *obj )
{
  return (tb_impl_t*)dtype_impl( obj_type( obj ) );
}

static inline size_t tb_minc( object_t *obj )
{
  return tb_impl( obj )->minc;
}

static inline type_t *tb_etype( object_t *obj )
{
  return tb_impl( obj )->entry_type;
}

static inline pad_t tb_pad( object_t *obj )
{
  return tb_impl( obj )->pad;
}

static inline sizefn_t tb_keysize( object_t *obj )
{
  return tb_impl( obj )->keysize;
}

static inline hashfn_t tb_keyhash( object_t *obj )
{
  return tb_impl( obj )->keyhash;
}

static inline compare_t tb_keycmp( object_t *obj )
{
  return tb_impl( obj )->keycmp;
}

static inline getter_t tb_getkey( object_t *obj )
{
  return tb_impl( obj )->getkey;
}

static inline getter_t tb_getvalue( object_t *obj )
{
  return tb_impl( obj )->getvalue;
}

static inline setter_t tb_setvalue( object_t *obj )
{
  return tb_impl( obj )->setvalue;
}

static inline intern_t tb_intern( object_t *obj )
{
  return tb_impl( obj )->intern;
}

static inline bool is_table( object_t *obj )
{
  return dtype_impl( obtype( obj ) ) != NULL
    && obtype( dtype_impl( obj ) ) == &TableImplType;
}

static inline bool is_set( object_t *obj )
{
  return is_table( obj )
    && tb_sig( obj )->length == 1;
}

static inline bool is_map( object_t *obj )
{
  return is_table( obj )
    && tb_sig( obj )->length == 2;
}

static inline type_t *tb_keytype( object_t *obj )
{
  return tb_sig( obj )->data[0];
}

static inline type_t *tb_valtype( object_t *obj )
{
  signature_t *sig = tb_sig( obj );

  return sig->data[sig->length-1];
}

static inline bool is_imap( object_t *obj ) // key is mapped to the entry, eg a symbol table
{
  return is_map( obj )
    && tb_valtype( obj ) == tb_etype( obj );
}

static inline size_t table_pad( object_t *obj, size_t n )
{
  return tb_impl( obj )->pad( tb_len( obj ),
			      n,
			      tb_cap( obj ),
			      tb_minc( obj ) );
}

#endif
