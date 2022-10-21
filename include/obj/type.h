#ifndef rascal_type_h
#define rascal_type_h

#include "rl/value.h"
#include "describe.h"

typedef struct type_t type_t;

// function pointer types
typedef bool        (*isa_t)( type_t *type, value_t value );
typedef bool        (*has_t)( type_t *type, type_t *sub );
typedef void        (*trace_t)( object_t *obj );
typedef void        (*free_t)( object_t *obj );
typedef void        (*init_t)( object_t *self, type_t *type, size_t n, void *data );
typedef rl_value_t  (*unwrap_t)( value_t val );
typedef value_t     (*wrap_t)( type_t *type, rl_value_t x );
typedef int         (*unbox_t)( void *dst, value_t src );
typedef object_t   *(*new_t)( type_t *type, size_t n );

// internal structure types
typedef enum
  {
    NTPTR,
    NTUINT,
    NTSINT,
    NTFLOAT
  } numtype_t;

// indicates the C type of the value and how it's boxed
typedef enum
  {
    SINT8, UINT8, ASCII, LATIN1, UTF8,

    SINT16, UINT16, UTF16, OPCODE, PRIMITIVE,

    SINT32, UINT32, UTF32, REAL32, CTYPE, BOOLEAN,

    NUL, FIXNUM, PTR, REAL, OBJ,
  } repr_t;

typedef struct
{
  uint8_t width:5; // size of the field in bytes
  uint8_t ctype:2; // pointer, unsigned, signed, or float
  uint8_t boxed:1; // boxed?
  uint8_t offset;  // offset from object base
} spec8_t;

typedef struct
{
  uint16_t width:13;
  uint16_t ctype:2;
  uint16_t boxed:1;
  uint16_t offset;
} spec16_t;

typedef struct
{
  uint32_t width:29;
  uint32_t ctype:2;
  uint32_t boxed:1;
  uint32_t offset;
} spec32_t;

typedef struct      // note: header for an array of either spec8, spec16, or spec32
{
  size_t base_size; // total size of all fixed fields 
  size_t n_fields;  // number of slots
} layout_t;

typedef struct
{
  layout_t layout;
  spec8_t  specs[];
} layout8_t;

typedef struct
{
  layout_t layout;
  spec16_t specs[];
} layout16_t;

typedef struct
{
  layout_t layout;
  spec32_t specs[];
} layout32_t;

typedef struct map_init_t map_init_t;

typedef struct
{
  Ctype_t   value;       // Ctype of the value representation (no tthe object Ctype)
  repr_t    repr;        // how the value is boxed (also provides builtin typecode)
  size_t    data_size;   // size of the value data (not the object size)
  value_t   tag;         // tag

  layout_t *layout;      // description of the object layout
  value_t   singleton;   // can be only value for type, or an empty collection
  object_t *slots;       // mapping from names to offsets
  object_t *signature;   // type information about constituents (layout varies)
  object_t *impl;        // low level interface methods, eg for table types. Optional.

  trace_t   trace;
  free_t    free;
  init_t    init;
  new_t     new;
  unwrap_t  unwrap;
  wrap_t    wrap;
  unbox_t   unbox;
} dtype_impl_t;

typedef struct
{
  object_t *members;
} utype_impl_t;

struct type_t
{
  OBJECT

  hash_t    hash;
  object_t *name;
  object_t *constructor;

  isa_t     isa;
  has_t     has;

  union
  {
    dtype_impl_t *dtype;
    utype_impl_t *utype;
  };
};

// fixed array of types, representing a signature for a function or type
typedef struct signature_t
{
  OBJECT

  size_t   length;
  size_t   capacity;
  type_t **data;
} signature_t;

// fixed set of types, representing a signature for a union type
typedef struct members_t
{
  OBJECT

  size_t   length;
  size_t   capacity;
  type_t **data;
} members_t;

// slots implementation
typedef struct slot_t
{
  OBJECT

  hash_t      hash;  // TODO: find a way around storing this?
  object_t   *name;  // symbolic name used to reference the slot
  fixnum_t    index; // used to look up field and type information
  object_t   *props; // metadata
} slot_t;

typedef struct slots_t
{
  OBJECT

  size_t   length;
  size_t   capacity;
  slot_t **data;
} slots_t;

#define N_IMM  (FIXNUM)
#define N_REPR (OBJ+1)

// constructor type
typedef enum
  {
    NOCONS=0,
    PRIMCONS=1,
    NATIVECONS=2
  } constype_t;

// globals
// fucked up types
extern type_t NoneType, AnyType, NulType;

// metaobject types
extern type_t DataType, UnionType, TypeType;

// internal types
extern type_t SignatureType, MembersType, SlotType, SlotsType;

// forward declarations
type_t *val_type( value_t val );
type_t *obj_type( object_t *obj );

bool rl_isa( value_t val, type_t *type );
bool isa_none( type_t *type, value_t val );
bool isa_singleton( type_t *type, value_t val );
bool isa_dtype( type_t *type, value_t val );
bool isa_utype( type_t *type, value_t val );

bool rl_has( type_t *self, type_t *other );
bool none_has( type_t *self, type_t *other );
bool any_has( type_t *self, type_t *other );
bool int_has( type_t *self, type_t *other );
bool utype_has( type_t *self, type_t *other );

rl_value_t rl_unwrap( value_t val );
rl_value_t fixnum_unwrap( value_t val );
rl_value_t real_unwrap( value_t val );
rl_value_t imm_unwrap( value_t val );
rl_value_t cv8_unwrap( value_t val );
rl_value_t cv16_unwrap( value_t val );
rl_value_t cv32_unwrap( value_t val );
rl_value_t cv64_unwrap( value_t val );

value_t rl_wrap( type_t *type, rl_value_t x );
value_t fixnum_wrap( type_t *type, rl_value_t x );
value_t nul_wrap( type_t *type, rl_value_t x );
value_t real_wrap( type_t *type, rl_value_t x );
value_t imm_wrap( type_t *type, rl_value_t x );
value_t cv8_wrap( type_t *type, rl_value_t x );
value_t cv16_wrap( type_t *type, rl_value_t x );
value_t cv32_wrap( type_t *type, rl_value_t x );
value_t cv64_wrap( type_t *type, rl_value_t x );

int rl_unbox( void *dst, value_t val );
int fixnum_unbox( void *dst, value_t val );
int nul_unbox( void *dst, value_t val );
int real_unbox( void *dst, value_t val );
int imm8_unbox( void *dst, value_t val );
int imm16_unbox( void *dst, value_t val );
int imm32_unbox( void *dst, value_t val );
int cv8_unbox( void *dst, value_t val );
int cv16_unbox( void *dst, value_t val );
int cv32_unbox( void *dst, value_t val );
int cv64_unbox( void *dst, value_t val );

void init_prim_dtype( type_t *type, char *name, object_t *constructor, size_t n, map_init_t *slots );
void init_prim_utype( type_t *type, char *name, object_t *constructor, size_t n, type_t **members );

// initialization
void rl_obj_type_init( void );
void rl_obj_type_mark( void );
void rl_obj_type_unmark( void );
void rl_obj_type_finalize( void );

// convenience
#define rl_type( x )          GENERIC_2( type, x )
#define is_type( x, T )       ( rl_type( x )==( T ) )

#define is_dtype( x )           is_type( x, &DataType )
#define is_utype( x )           is_type( x, &UnionType )

#define as_type( x )          ( (type_t*)as_obj( x ) )

#define obtype( x )           ( (type_t*)hdr_data(as_obj(x)->type) )
#define obtrace( x )          ( type_trace( obtype( x ) ) )
#define obdtype( x )          ( obtype( x )->dtype )

#define type_name( t )        ( as_type( t )->name )
#define type_constructor( t ) ( as_type( t )->constructor )
#define type_hash( t )        ( as_type( t )->hash )
#define type_isa( t )         ( as_type( t )->isa )
#define type_has( t )         ( as_type( t )->has )
#define type_dtype( t )       ( as_type( t )->dtype )
#define type_utype( t )       ( as_type( t )->utype )

#define dtype_value( t )      ( type_dtype( t )->value )
#define dtype_data_size( t )  ( type_dtype( t )->data_size )
#define dtype_repr( t )       ( type_dtype( t )->repr )
#define dtype_tag( t )        ( type_dtype( t )->tag )
#define dtype_layout( t )     ( type_dtype( t )->layout )
#define dtype_singleton( t )  ( type_dtype( t )->singleton )
#define dtype_slots( t )      ( type_dtype( t )->slots )
#define dtype_signature( t )  ( type_dtype( t )->signature )
#define dtype_impl( t )       ( type_dtype( t )->impl )
#define dtype_trace( t )      ( type_dtype( t )->trace )
#define dtype_free( t )       ( type_dtype( t )->free )
#define dtype_init( t )       ( type_dtype( t )->init )
#define dtype_unwrap( t )     ( type_dtype( t )->unwrap )
#define dtype_wrap( t )       ( type_dtype( t )->wrap )
#define dtype_unbox( t )      ( type_dtype( t )->unbox )

#define as_layout8( l )       ( (layout8_t*)(l) )
#define as_layout16( l )      ( (layout16_t*)(l) )
#define as_layout32( l )      ( (layout32_t*)(l) )

#define utype_members( t )    ( type_utype( t )->members )

#define as_slot( x )          ( (slot_t*)as_obj( x ) )
#define slot_name( x )        ( as_slot( x )->name )
#define slot_index( x )       ( as_slot( x )->index )
#define slot_hash( x )        ( as_slot( x )->hash )
#define slot_props( x )       ( as_slot( x )->props )

static inline bool is_obj_type( type_t *type )
{
  return obtype( type ) == &DataType && dtype_value( type ) == Ctype_object;
}

static inline bool is_imm_type( type_t *type )
{
  return obtype( type ) == &DataType && dtype_value( type ) != Ctype_object;
}

#endif
