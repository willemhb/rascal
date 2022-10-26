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
  OBJECT;

  size_t    minc;
  type_t   *etype;
  pad_t     pad;
  hashfn_t  hashkey;
  compare_t cmpkeys;
  getter_t  gethash;
  getter_t  getkey;
  getter_t  getvalue;
  setter_t  setvalue;
  intern_t  intern;
} tb_impl_t;

// globals
extern type_t TableImplType;
extern layout_t TableLayout;

// forward declarations
size_t     pad_table_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
void       trace_table( object_t *object );
void       free_table( object_t *object );
void       resize_table( object_t *object, size_t newl );
object_t  *create_table( type_t *type, size_t n );
object_t **rehash_table( object_t *object, object_t **news, size_t newc );

bool table_assc( object_t *obj, rl_value_t *buf, rl_value_t key );
bool table_intern( object_t *obj, rl_value_t *buf, rl_value_t key );
bool table_popi( object_t *obj, rl_value_t *buf, rl_value_t key );
bool table_has( object_t *obj, rl_value_t key);
bool table_is_superset( object_t *self, object_t *other );
bool table_is_subset( object_t *self, object_t *other );
// generally useful getters
rl_value_t id_getkey( object_t *obj );
rl_value_t id_getval( object_t *obj );
int        id_cmpkey( rl_value_t x, rl_value_t y );
hash_t     id_hashkey( rl_value_t x );
object_t  *id_intern( object_t *obj, rl_value_t k, hash_t h, size_t j );    // intern the key itself (key should store its own hash)
object_t  *etype_intern( object_t *obj, rl_value_t k, hash_t h, size_t j ); // create a new entry (standard)

// silly initialization stuff
void rl_init_obj_table( void );
void rl_mark_obj_table( void );
void rl_unmark_obj_table( void );
void rl_finalize_obj_table( void );

// convenience
#define as_table( x )   ( (table_t*)as_obj( x ) )
#define tb_len( x )     ( as_table( x )->length )
#define tb_cap( x )     ( as_table( x )->capacity )
#define tb_data( x )    ( as_table( x )->data )

#endif
