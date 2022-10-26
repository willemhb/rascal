#ifndef rascal_type_h
#define rascal_type_h

#include "rl/value.h"
#include "describe.h"

typedef struct type_t type_t;
typedef struct object_impl_t object_impl_t;
typedef struct array_impl_t array_impl_t;
typedef struct table_impl_t table_impl_t;
typedef struct atom_impl_t  atom_impl_t;
typedef struct cvalue_impl_t cvalue_impl_t;

// function pointer types
typedef bool        (*isa_t)( type_t *type, value_t value );
typedef bool        (*has_t)( type_t *type, type_t *sub );
typedef hash_t      (*hashfn_t)( rl_value_t val );
typedef int         (*compare_t)( rl_value_t x, rl_value_t y );

// internal structure types for representing type information
typedef struct map_init_t map_init_t;

struct type_t
{
  OBJECT;

  hash_t    hash;
  object_t *name;
  object_t *constructor;

  isa_t     isa;
  has_t     has;

  union
  {
    struct
    {
      value_t   tag;
      value_t   singleton;   // can be only value for type, or an empty collection
      hashfn_t  hashfn;      // called to has a member of this type
      compare_t compare;     // called to compare two members of this type

      object_t *cvalue_impl;
      object_t *object_impl;
      object_t *atom_impl;
      object_t *table_impl;
      object_t *array_impl;
    } dtype;

    struct
    {
      object_t *members;
    } utype;
  };
};

// fixed set of types, representing a signature for a union type
typedef struct members_t
{
  OBJECT;

  size_t   length;
  size_t   capacity;
  type_t **data;
} members_t;

// globals
// fucked up types
extern type_t NoneType, AnyType;

// metaobject types
extern type_t DataType, UnionType, TypeType;

// internal types
extern type_t MembersType;

// forward declarations
type_t *val_type( value_t val );
type_t *obj_type( object_t *obj );

bool rl_isa( value_t val, type_t *type );
bool singleton_isa( type_t *type, value_t val );
bool dtype_isa( type_t *type, value_t val );
bool utype_isa( type_t *type, value_t val );

bool rl_has( type_t *self, type_t *other );
bool none_has( type_t *self, type_t *other );
bool int_has( type_t *self, type_t *other );
bool utype_has( type_t *self, type_t *other );

void init_prim_dtype( type_t *type, char *name, object_t *constructor, size_t n, map_init_t *slots );
void init_prim_utype( type_t *type, char *name, object_t *constructor, size_t n, type_t **members );

// initialization
void rl_init_obj_type( void );
void rl_mark_obj_type( void );
void rl_unmark_obj_type( void );
void rl_finalize_obj_type( void );

// convenience
#define rl_type( x )          GENERIC_2( type, x )
#define is_type( x, T )       ( rl_type( x )==( T ) )

#define is_dtype( x )           is_type( x, &DataType )
#define is_utype( x )           is_type( x, &UnionType )

#define as_type( x )          ( (type_t*)as_obj( x ) )

#define obtype( x )           ( (type_t*)hdr_data( as_obj(x)->type ) )
#define obtrace( x )          ( type_trace( obtype( x ) ) )
#define obdtype( x )          ( obtype( x )->dtype )

#define type_name( t )         ( as_type( t )->name )
#define type_constructor( t )  ( as_type( t )->constructor )
#define type_hash( t )         ( as_type( t )->hash )
#define type_isa( t )          ( as_type( t )->isa )
#define type_has( t )          ( as_type( t )->has )
#define type_dtype( t )        ( as_type( t )->dtype )
#define type_utype( t )        ( as_type( t )->utype )

#define type_tag( t )          ( type_dtype( t ).tag )
#define type_layout( t )       ( type_dtype( t ).layout )
#define type_singleton( t )    ( type_dtype( t ).singleton )
#define type_slots( t )        ( type_dtype( t ).slots )
#define type_signature( t )    ( type_dtype( t ).signature )
#define type_object_impl( t )  ( (object_impl_t*)type_dtype( t ).object_impl )
#define type_cvalue_impl( t )  ( (cvalue_impl_t*)type_dtype( t ).cvalue_impl )
#define type_atom_impl( t )    ( (atom_impl_t*)type_dtype( t ).atom_impl )
#define type_table_impl( t )   ( (table_impl_t*)type_dtype( t ).table_impl )
#define type_array_impl( t )   ( (array_impl_t*)type_dtype( t ).array_impl )

#define type_members( t )      ( type_utype( t ).members )

#define is_object_type( t )    ( type_object_impl( t ) != NULL )
#define is_immediate_type( t ) ( type_object_impl( t ) == NULL )
#define is_cvalue_type( t )    ( type_cvalue_impl( t ) != NULL )
#define is_table_type( t )     ( type_table_impl( t )  != NULL )
#define is_array_type( t )     ( type_array_impl( t )  != NULL )

#define as_slot( x )          ( (slot_t*)as_obj( x ) )
#define slot_name( x )        ( as_slot( x )->name )
#define slot_index( x )       ( as_slot( x )->index )
#define slot_hash( x )        ( as_slot( x )->hash )
#define slot_props( x )       ( as_slot( x )->props )

#endif
