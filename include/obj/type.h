#ifndef rascal_type_h
#define rascal_type_h

#include "rl/value.h"
#include "describe.h"

typedef struct type_t type_t;

// function pointer types
typedef bool      (*isa_t)( type_t *type, value_t value );
typedef bool      (*has_t)( type_t *type, type_t *sub );
typedef void      (*trace_t)( object_t *obj );
typedef void      (*free_t)( object_t *obj );
typedef void      (*init_t)( type_t *type, object_t *self, size_t n, void *data );
typedef object_t *(*new_t)( type_t *type, size_t n );

// internal structure types
typedef enum
  {
    NTPTR,
    NTUNSIGNED,
    NTSIGNED,
    NTFLOAT
  } numtype_t;

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
  OBJECT

  hash_t    hash;
  size_t    index; // used to look up field and type information
  value_t   name;
  object_t *props; // metadata
} slot_t;

typedef struct
{
  Ctype_t   value;       // Ctype of the value representation (no tthe object Ctype)
  arity_t   data_size;   // size of the value data (not the object size)
  bool      fits_fixnum; // indicates that the unboxed data fits in a fixnum
  bool      fits_word;   // indicates that the unboxed data fits in a word

  layout_t *layout;      // description of the object layout
  value_t   singleton;   // can be only value for type, or an empty collection
  object_t *slots;       // mapping from names to offsets
  object_t *types;       // signature; layout depends on type

  trace_t   trace;
  free_t    free;
  init_t    init;
  new_t     new;
} dtype_impl_t;

typedef struct
{
  object_t *members;
} utype_impl_t;

struct type_t
{
  OBJECT

  object_t *name;
  object_t *constructor;
  hash_t    hash;

  isa_t     isa;
  has_t     has;

  union
  {
    dtype_impl_t *dtype;
    utype_impl_t *utype;
  };
};

// immediate typecodes
typedef enum
  {
    SINT8, UINT8, ASCII, LATIN1, UTF8,

    SINT16, UINT16, UTF16, OPCODE, PRIMITIVE,

    SINT32, UINT32, UTF32, REAL32, CTYPE, BOOLEAN,

    NUL,

    N_IMM
  } immtype_t;

// constructor type
typedef enum
  {
    NOCONS=0,
    PRIMCONS=1,
    NATIVECONS=2
  } constype_t;

// globals
// immediate types
extern type_t *FixnumType, *RealType, *PointerType, *ImmTypes[N_IMM];

// fucked up types
extern type_t *NoneType, *AnyType, *NulType;

// metaobject types
extern type_t *DataType, *UnionType;

// forward declarations
type_t *val_type( value_t val );
type_t *obj_type( object_t *obj );

bool isa_none( object_t *object, value_t val );
bool isa_any( object_t *object, value_t val );
bool isa_dtype( object_t *object, value_t val );
bool isa_utype( object_t *object, value_t val );

bool none_has( object_t *self, object_t *other );
bool any_has( object_t *self, object_t *other );
bool dtype_has( object_t *self, object_t *other );
bool utype_has( object_t *self, object_t *other );

type_t *init_dtype( type_t *type, char *name, constype_t constype, size_t n, ... );
type_t *init_utype( type_t *type, char *name, constype_t constype, size_t n, ... );

// convenience
#define rl_type( x )    GENERIC_2( type, x )
#define is_type( x, T ) (rl_type(x)==(T))
#define obtype( x )     ((type_t*)hdr_data(as_obj(x)->type))

static inline bool is_dtype( type_t *type )
{
  return obtype(type) == DataType;
}

static inline bool is_utype( type_t *type )
{
  return obtype(type) == UnionType;
}

static inline bool is_obj_type( type_t *type )
{
  return obtype(type) == DataType && !!type->dtype->layout;
}

static inline bool is_imm_type( type_t *type )
{
  return obtype(type) == DataType && type->dtype->layout == NULL;
}

#endif
