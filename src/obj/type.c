#include <string.h>

#include "obj/type.h"
#include "obj/table.h"
#include "obj/array.h"
#include "obj/object.h"
#include "obj/symbol.h"
#include "obj/dispatch.h"
#include "obj/cvalue.h"
#include "obj/number.h"

#include "utils/hashing.h"

// type implementations
// data-type type
#define DTYPE_N 14

extern object_impl_t DataTypeObjImpl;

hash_t hash_dtype( rl_value_t x );
int    compare_dtypes( rl_value_t x, rl_value_t y );

type_t DataType =
  {
   .obj = { .dtype=&DataType, .hdrtag=HDR_BITS },

   .isa = dtype_isa,
   .has = none_has,

   .dtype =
   {
    .tag        =OBJ_TAG,
    .hashfn     =hash_dtype,
    .compare    =compare_dtypes,
    .object_impl=&DataTypeObjImpl.obj,
   }
  };

extern layout_t DataTypeLayout;
extern signature_t DataTypeSig;
extern slots_t DataTypeSlots;

void trace_datatype( object_t *obj );
void init_datatype( object_t *self, type_t *type, size_t n, void *ini );

object_impl_t DataTypeObjImpl =
  {
   .obj      = { .dtype=&ObjectImplType, .hdrtag=HDR_BITS },

   .layout   = &DataTypeLayout,
   .signature= &DataTypeSig.obj,
   .slots    = &DataTypeSlots.obj,

   .trace    = trace_datatype,
   .init     = init_datatype,
   .create   = create_fixobj
  };

layout_t DataTypeLayout =
  {
   .base_size=sizeof(type_t), .n_fields=DTYPE_N,

   {
    { sizeof(hash_t),    Ctype_uint64,  false, offsetof(type_t, hash )              },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, name )              },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, constructor )       },
    { sizeof(isa_t),     Ctype_pointer, false, offsetof(type_t, isa )               },
    { sizeof(has_t),     Ctype_pointer, false, offsetof(type_t, has )               },
    { sizeof(value_t),   Ctype_uint64,  false, offsetof(type_t, dtype.tag )         },
    { sizeof(value_t),   Ctype_uint64,  true,  offsetof(type_t, dtype.singleton )   },
    { sizeof(hashfn_t),  Ctype_pointer, false, offsetof(type_t, dtype.hashfn )      },
    { sizeof(compare_t), Ctype_pointer, false, offsetof(type_t, dtype.compare)      },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, dtype.cvalue_impl ) },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, dtype.object_impl ) },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, dtype.atom_impl )   },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, dtype.table_impl )  },
    { sizeof(object_t*), Ctype_object,  false, offsetof(type_t, dtype.array_impl )  }
   }
  };

extern type_t *DataTypeSigData[DTYPE_N];

signature_t DataTypeSig =
  {
   .obj     = { .dtype=&SignatureType, .hdrtag=HDR_BITS },
   .length  =DTYPE_N,
   .capacity=DTYPE_N,
   .data    =DataTypeSigData
  };

type_t *DataTypeSigData[DTYPE_N] =
  {
   &UInt64Type,     &SymbolType,     &FunctionType,
   
   &PointerType,    &PointerType,    &UInt64Type,

   &AnyType,        &PointerType,    &PointerType,

   &CvalueImplType, &ObjectImplType, &AtomImplType,

   &TableImplType,  &ArrayImplType,
  };

void trace_datatype( object_t *obj )
{
  mark_obj( type_name( obj ) );
  mark_obj( type_constructor( obj ) );
  mark_val( type_singleton( obj ) );
  mark_obj( type_dtype( obj ).cvalue_impl );
  mark_obj( type_dtype( obj ).object_impl );
  mark_obj( type_dtype( obj ).atom_impl );
  mark_obj( type_dtype( obj ).table_impl );
  mark_obj( type_dtype( obj ).array_impl );
}

// union type
#define UTYPE_N 6ul

extern object_impl_t UnionTypeImpl;

hash_t hash_utype( rl_value_t val );
int    compare_utypes( rl_value_t x, rl_value_t y );

type_t UnionType =
  {
   .obj = { .dtype=&DataType, .hdrtag=HDR_BITS },

   .isa  = utype_isa,
   .has  = utype_has,
   .dtype=
   {
    .tag         = OBJ_TAG,
    .hashfn      = hash_utype,
    .compare     = compare_utypes,
    .object_impl = &UnionTypeImpl.obj
   }
  };

// external API functions
type_t *val_type( value_t val )
{
  switch (val_tag(val))
    {
    case IMM_TAG: return ImmTypes[imm_tag(val)];
    case PTR_TAG: return &PointerType;
    case FIX_TAG: return &FixnumType;
    case OBJ_TAG: return obj_type( as_obj(val) );
    default:      return &RealType;
    }
}

type_t *obj_type( object_t *obj )
{
  assert( obj );
  return (type_t*)hdr_data(obj->type);
}


// mark and unmark functions
void rl_obj_type_mark( void )
{
  mark_obj( &NoneType.obj );
  mark_obj( &AnyType.obj );
  mark_obj( &DataType.obj );
  mark_obj( &UnionType.obj );
  mark_obj( &TypeType.obj );
  mark_obj( &SignatureType.obj );
  mark_obj( &MembersType.obj );
  mark_obj( &SlotType.obj );
  mark_obj( &SlotsType.obj );
}

void rl_obj_type_unmark( void )
{
  unmark_obj( &NoneType.obj );
  unmark_obj( &AnyType.obj );
  unmark_obj( &DataType.obj );
  unmark_obj( &UnionType.obj );
  unmark_obj( &TypeType.obj );
  unmark_obj( &SignatureType.obj );
  unmark_obj( &MembersType.obj );
  unmark_obj( &SlotType.obj );
  unmark_obj( &SlotsType.obj );  
}

void rl_obj_type_init( void )
{
}


// globals & type implementations implementation
// type hashing

// type initialization
void init_prim_dtype( type_t *type, char *name, object_t *constructor, size_t n, map_init_t *slots )
{
}

void init_prim_utype( type_t *type, char *name, object_t *constructor, size_t n, type_t **members )
{
}


// isa implementations
bool rl_isa( value_t val, type_t *type )
{
  return type->isa( type, val );
}

bool none_isa( type_t *type, value_t val )
{
  (void)type;
  (void)val;
  return false;
}

bool any_isa( type_t *type, value_t val )
{
  (void)type;
  (void)val;
  return true;
}

bool singleton_isa( type_t *type, value_t val )
{
  return type->dtype->singleton == val;
}

inline bool dtype_isa( type_t *type, value_t val )
{
  return val_type( val ) == type;
}

inline bool utype_isa( type_t *type, value_t val )
{
  return set_has( type->utype->members, (rl_value_t)val_type( val ) );
}

bool rl_has( type_t *self, type_t *other )
{
  return self->has( self, other );
}

bool none_has( type_t *self, type_t *other )
{
  (void)self;
  (void)other;
  return false;
}

bool any_has( type_t *self, type_t *other )
{
  (void)self;
  (void)other;
  return true;
}

bool int_has( type_t *self, type_t *other )
{
  Ctype_t Cself = dtype_value( self  ), Cother = dtype_value( other );

  return Cself != Cother && common_Ctype(Cself, Cother) == Cself;
}

bool utype_has( type_t *self, type_t *other )
{
  if ( other == &NoneType )
    return true;

  if ( other == &AnyType )
    return false;

  if ( is_dtype( &other->obj ) )
    return set_has( utype_members( self ), (rl_value_t)other );
  
  return set_sup( utype_members( self ), utype_members( other ) );
}
