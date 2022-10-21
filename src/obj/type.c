#include <string.h>

#include "obj/type.h"
#include "obj/table.h"
#include "obj/array.h"
#include "obj/cvalue.h"
#include "obj/symbol.h"
#include "obj/function.h"
#include "vm/memory.h"
#include "utils/hashing.h"
#include "obj/cell.h"
#include "obj/native.h"

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

// isa implementations
bool rl_isa( value_t val, type_t *type )
{
  return type->isa( type, val );
}

bool isa_none( type_t *type, value_t val )
{
  (void)type;
  (void)val;
  return false;
}

bool isa_any( type_t *type, value_t val )
{
  (void)type;
  (void)val;
  return true;
}

bool isa_singleton( type_t *type, value_t val )
{
  return type->dtype->singleton == val;
}

inline bool isa_dtype( type_t *type, value_t val )
{
  return val_type( val ) == type;
}

inline bool isa_utype( type_t *type, value_t val )
{
  return set_has( type->utype->members, (rl_value_t)val_type( val ) );
}

bool rl_has( type_t *self, type_t *other )
{
  return self->has( self, other );
}

bool has_none( type_t *self, type_t *other )
{
  (void)self;
  (void)other;
  return false;
}

bool has_any( type_t *self, type_t *other )
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

// toplevel dispatch for wrapping/unwrapping
rl_value_t rl_unwrap( value_t val )
{
  return val_type( val )->dtype->unwrap( val );
}

value_t rl_wrap( type_t *type, rl_value_t x )
{
  return type->dtype->wrap( type, x );
}

int rl_unbox( void *spc, value_t val )
{
  return val_type( val )->dtype->unbox( spc, val );
}

// unwrap implementations
#define cv_unwrap( size )					\
  rl_value_t cv##size##_unwrap( value_t val )			\
  {								\
    return (rl_value_t)(*(uint##size##_t*)(as_obj(val)+1));	\
  }

rl_value_t fixnum_unwrap( value_t val )
{
  return (rl_value_t)(val&PTR_MASK);
}

rl_value_t real_unwrap( value_t val )
{
  return (rl_value_t)val;
}

rl_value_t imm_unwrap( value_t val )
{
  return (rl_value_t)(val&IMM_MASK);
}

cv_unwrap( 8 );
cv_unwrap( 16 );
cv_unwrap( 32 );
cv_unwrap( 64 );

// wrap implemenations
#define cv_wrap( size )						\
  value_t cv##size##_wrap( type_t *type, rl_value_t x )		\
  {								\
    uint##size##_t buffer = x.as_uint##size;			\
    object_t *wrapper     = new_obj( type, size>>3, &buffer );	\
    return (value_t)wrapper | type->dtype->tag;			\
  }

value_t fixnum_wrap( type_t *type, rl_value_t x )
{
  return (x.as_fixnum&PTR_MASK)|type->dtype->tag;
}

value_t real_wrap( type_t *type, rl_value_t x )
{
  (void)type;
  return x.as_data;
}

value_t nul_wrap( type_t *type, rl_value_t x )
{
  (void)x;
  return type->dtype->singleton;
}

value_t imm_wrap( type_t *type, rl_value_t x )
{
  return x.as_data|type->dtype->tag|((value_t)type->dtype->repr<<32);
}

cv_wrap( 8 );
cv_wrap( 16 );
cv_wrap( 32 );
cv_wrap( 64 );

// box implementations
int fixnum_unbox( void *dst, value_t val )
{
  *(fixnum_t*)dst = fixnum_unwrap( val ).as_fixnum;
  return sizeof(fixnum_t);
}

int real_unbox( void *dst, value_t val )
{
  *(real_t*)dst = as_real( val );
  return sizeof(real_t);
}

int nul_unbox( void *dst, value_t val )
{
  assert( val ==NUL_VAL );
  *(void**)dst = NULL;
  return 8;
}

#define imm_unbox( size )					\
  int imm##size##_unbox( void *dst, value_t val )		\
  {								\
    *(uint##size##_t*)dst = imm_unwrap(val).as_uint##size;	\
    return size>>3;						\
  }

#define cv_unbox( size )						\
  int cv##size##_unbox( void *dst, value_t val )			\
  {									\
    *(uint##size##_t*)dst = cv##size##_unwrap(val).as_uint##size;	\
    return size>>3;							\
  }

imm_unbox( 8 );
imm_unbox( 16 );
imm_unbox( 32 );
cv_unbox( 8 );
cv_unbox( 16 );
cv_unbox( 32 );
cv_unbox( 64 );

// globals & type implementations implementation
// type hashing
hash_t hash_dtype( type_t *type )
{
  hash_t base      = sym_hash( type_name( type ) );
  layout_t *layout = dtype_layout( type );

  if ( layout == NULL ) // immediate types, &c
    {
      base = mix_hashes( base, dtype_repr( type ) );

      if ( dtype_singleton( type ) != 0 )
	base = mix_hashes( base, dtype_singleton( type ) );

      return base;
    }

  byte *fields; size_t total;

  if ( layout->base_size < UINT8_MAX )
    {
      fields = (byte*)as_layout8( layout )->specs;
      total  = layout->n_fields * sizeof(spec8_t);
    }

  else if ( layout->base_size < UINT16_MAX )
    {
      fields = (byte*)as_layout16( layout )->specs;
      total  = layout->n_fields * sizeof(spec16_t);
    }

  else
    {
      fields   = (byte*)as_layout32( layout )->specs;
      total    = layout->n_fields * sizeof(spec32_t);
    }

  hash_t fieldhash = hash_bytes( fields, total );
  return mix_hashes( base, fieldhash );
}

hash_t hash_utype( object_t *name, size_t n, type_t **types )
{
  hash_t base = sym_hash( name );

  if ( types == NULL )                // special cases of Any/None/Nul. Shouldn't clash with anything?
    return mix_hashes( base, base );

  for (size_t i=0; i<n; i++)
    base = mix_hashes( base, type_hash( types[i] ) );

  return base;
}

// signature type
#define SIGNATURE_MINC 0

arr_impl_t SignatureArrImpl =
  {
    { .dtype=&ArrayImplType },

    .elsize=sizeof(type_t*),
    .Ctype =Ctype_object,
    .minc  =0,

    .pad     =NULL,
    .aref    =rl_object_aref,
    .aset    =NULL,
    .appendto=NULL
  };

type_t *SignatureSignatureData[] = { &TypeType };

signature_t SignatureSignature =
  {
    { .dtype = &SignatureType },

    .length=1, .capacity=1,
    .data = SignatureSignatureData
  };

signature_t EmptySignature =
  {
    { .dtype= &SignatureType },
    .length=0, .capacity=0,
    .data  =NULL
  };

dtype_impl_t SignatureDtypeImpl =
  {
    .value=Ctype_object, .repr=OBJ, .data_size=sizeof(signature_t*), .tag  =OBJ_TAG,

    .layout   =&ArrayLayout.layout,
    .singleton=(value_t)&EmptySignature,
    .signature=&SignatureSignature.obj,
    .trace    =trace_objects,
    .free     =free_array,
    .init     =init_fixed_array,
    .new      =new_immutable_array,
    .unwrap   =fixnum_unwrap,
    .wrap     =fixnum_wrap,
    .unbox    =fixnum_unbox,
    .impl     =&SignatureArrImpl.obj
  };

type_t SignatureType =
  {
    { .dtype =&DataType, .permanent=true },

    .isa=isa_dtype,
    .has=none_has,
    .dtype=&SignatureDtypeImpl
  };

// members type
#define MEMBERS_MINC 0

hash_t members_keyhash( rl_value_t key )
{
  return type_hash( key.as_type );
}

type_t *MembersSigData[] = { &TypeType };

signature_t MembersSig =
  {
    { .dtype=&SignatureType, .permanent=true },

    .length  =1,
    .capacity=1,
    .data    =MembersSigData
  };

tb_impl_t MembersTbImpl =
  {
    { .dtype=&TableImplType, .permanent=true },
    
    .minc=MEMBERS_MINC,
    .entry_type=&TypeType,
    .pad=pad_table_size,
    .keyhash=members_keyhash,
    .getkey=id_getkey,
    .getvalue=id_getval,
    .keycmp=id_keycmp,
    .intern=id_intern
  };

dtype_impl_t MembersDtImpl =
  {
    .value    =Ctype_object,
    .data_size=sizeof(object_t*),
    .repr     =OBJ,
    .trace    =trace_table,
    .free     =free_table,
    .init     =init_set,
    .new      =new_table,
    .layout   =&TableLayout.layout,
    .unwrap   =fixnum_unwrap,
    .wrap     =fixnum_wrap,
    .unbox    =fixnum_unbox,
    .signature=&MembersSig.obj,
    .impl     =&MembersTbImpl.obj
  };

type_t MembersType =
  {
    .obj = { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa  =isa_dtype,
    .has  =none_has,
    .dtype=&MembersDtImpl
  };

// slot type

layout8_t SlotLayout =
  {
    { .base_size=sizeof(slot_t), .n_fields=4 },
    {
      { sizeof(hash_t),    NTUINT, false, offsetof( slot_t, hash)   }, 
      { sizeof(fixnum_t),  NTUINT, false, offsetof( slot_t, index ) },
      { sizeof(object_t*), NTPTR,  false, offsetof( slot_t, name )  },
      { sizeof(object_t*), NTPTR,  false, offsetof( slot_t, props ) }
    }
  };

type_t *SlotSigData[] =
  {
    &UInt64Type, &FixnumType, &SymbolType, &ConsType
  };

signature_t SlotSig =
  {
    .obj     = { .dtype=&SignatureType, .permanent=true, .hdrtag=HDR_BITS },
    .length  = 4,
    .capacity= 4,
    .data    = SlotSigData
  };

void trace_slot( object_t *obj )
{
  mark_objs( &slot_name( obj ), 2 );
}

void init_slot( object_t *self, type_t *type, size_t n, void *ini )
{
  (void)type;
  (void)n;

  map_init_t m_ini = *(map_init_t*)ini;

  slot_hash( self )  = m_ini.hash;
  slot_name( self )  = m_ini.key.as_object;
  slot_index( self ) = m_ini.val.as_fixnum;
  slot_props( self ) = NULL;
}

slots_t SlotsSlots =
  {
    .obj     = { .dtype=&SlotsType, .permanent=true, .hdrtag=HDR_BITS },
    .length  = 0,
    .capacity= 8,
    .data    = NULL
  };

dtype_impl_t SlotDtImpl =
  {
    .value    =Ctype_object,
    .data_size=sizeof(object_t*),
    .repr     =OBJ,
    .tag      =OBJ_TAG,
    .layout   =&SlotLayout.layout,
    .new      =new_fobj,
    .init     =init_slot,
    .trace    =trace_slot,
    .wrap     =fixnum_wrap,
    .unwrap   =fixnum_unwrap,
    .unbox    =fixnum_unbox,
    .signature=&SlotSig.obj,
    .slots    =&SlotsSlots.obj
  };

type_t SlotType =
  {
    .obj   = { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa   = isa_dtype,
    .has   = none_has,
 
    .dtype = &SlotDtImpl
  };

// slots type
#define SLOTS_MINC 4
rl_value_t slot_getkey( object_t *obj )
{
  return (rl_value_t)slot_name( obj );
}

rl_value_t slot_getval( object_t *obj )
{
  return (rl_value_t)obj;
}

hash_t slot_keyhash( rl_value_t key )
{
  return sym_hash( key.as_object );
}

object_t *slot_intern( object_t *obj, rl_value_t key, hash_t h, size_t j )
{  
  map_init_t m   = { .key=key, .hash=h, .val=(rl_value_t)tb_len( obj ) };

  return ( tb_data( obj )[j] = new_obj( tb_etype( obj ), 0, &m) );
}

type_t *SlotsSigData[] = { &SymbolType, &SlotType };

signature_t SlotsSig =
  {
    .obj     ={ .dtype=&SignatureType, .permanent=true, .hdrtag=HDR_BITS },
    .length  =2,
    .capacity=2,
    .data    =SlotsSigData
  };

tb_impl_t SlotsImpl =
  {
    .obj={ .dtype=&TableImplType, .permanent=true, .hdrtag=HDR_BITS },
    .minc=SLOTS_MINC,
    .entry_type=&SlotType,
    .pad=pad_table_size,
    .keysize=id_keysize,
    .keyhash=slot_keyhash,
    .keycmp=id_keycmp,
    .getkey=slot_getkey,
    .getvalue=slot_getval,
    .intern=slot_intern
  };

dtype_impl_t SlotsDtImpl =
  {
    .value    =Ctype_object,
    .data_size=sizeof(object_t*),
    .repr     =OBJ,
    .tag      =OBJ_TAG,
    .layout   =&TableLayout.layout,
    .signature=&SlotsSig.obj,
    .impl     =&SlotsImpl.obj,
    .trace    =trace_table,
    .free     =free_table,
    .init     =init_set,
    .new      =new_table,
    .unwrap   =fixnum_unwrap,
    .wrap     =fixnum_wrap,
    .unbox    =fixnum_unbox
  };

type_t SlotsType =
  {
    { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa=isa_dtype,
    .has=none_has,
    .dtype=&SlotsDtImpl
  };

// fucked up types
type_t AnyType =
  {
    { .dtype=&UnionType, .permanent=true, .hdrtag=HDR_BITS },

    .isa=isa_any,
    .has=any_has,
    .utype=NULL
  };

type_t NoneType =
  {
    { .dtype=&UnionType, .permanent=true, .hdrtag=HDR_BITS },

    .isa  =isa_none,
    .has  =none_has,
    .utype=NULL
  };

dtype_impl_t NulTypeImpl =
  {
    .value    =Ctype_pointer,
    .repr     =NUL,
    .data_size=sizeof(void*),
    .singleton=NUL_VAL,
    .unwrap   =fixnum_unwrap,
    .wrap     =nul_wrap,
    .unbox    =nul_unbox
  };

type_t NulType =
  {
    { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa=isa_singleton,
    .has=none_has,
    .dtype=&NulTypeImpl
  };

// metaobject type implementations
void trace_dtype( object_t *obj )
{
  type_t *type = (type_t*)obj;

  obj_mark( type_name( type ) );
  obj_mark( type_constructor( type ) );
  val_mark( dtype_singleton( type ) );
  obj_mark( dtype_slots( type ) );
  obj_mark( dtype_signature( type ) );
  obj_mark( dtype_impl( type ) );
}

void trace_utype( object_t *obj )
{
  obj_mark( type_name( obj ) );
  obj_mark( type_constructor( obj ) );
  obj_mark( utype_members( obj ) ); 
}

void init_dtype( object_t *self, type_t *type, size_t n, void *data )
{
  (void)self;
  (void)type;
  (void)n;
  (void)data;
}

void init_utype( object_t *self, type_t *type, size_t n, void *data )
{
  (void)self;
  (void)type;
  (void)n;
  (void)data;
}

void free_dtype( object_t *obj )
{
  layout_t *layout = dtype_layout( obj );

  if ( layout->base_size < UINT8_MAX )
    dealloc( layout, sizeof(layout_t)+sizeof(spec8_t)*layout->n_fields );

  else if ( layout->base_size < UINT16_MAX )
    dealloc( layout, sizeof(layout_t)+sizeof(spec16_t)*layout->n_fields );

  else
    dealloc( layout, sizeof(layout_t)+sizeof(spec32_t)*layout->n_fields );

  dealloc( type_dtype( obj ), sizeof(dtype_impl_t) );
}

void free_utype( object_t *obj )
{
  type_t *type =(type_t*)obj;

  dealloc( type->utype, sizeof(utype_impl_t) );
}

type_t *TypeSignatureData[] =
  {
    &SymbolType,  &FunctionType, &UInt64Type,
    &PointerType, &PointerType,  &PointerType
  };

signature_t TypeSignature =
  {
    { .dtype=&SignatureType, .permanent=true, .hdrtag=HDR_BITS },
    .length = 6, .capacity = 6, .data = TypeSignatureData
  };

layout8_t TypeLayout =
  {
    { .base_size=sizeof(type_t), .n_fields=6 },
    {
      { sizeof(object_t*), NTPTR,  false, offsetof(type_t, name)        },
      { sizeof(object_t*), NTPTR,  false, offsetof(type_t, constructor) },
      { sizeof(hash_t),    NTUINT, false, offsetof(type_t, hash)        },
      { sizeof(isa_t),     NTPTR,  false, offsetof(type_t, isa)         },
      { sizeof(has_t),     NTPTR,  false, offsetof(type_t, has)         },
      { sizeof(void*),     NTPTR,  false, offsetof(type_t, dtype)       }
    }
  };

dtype_impl_t DataTypeImpl =
  {
    .value=Ctype_object, .repr=OBJ, .data_size=sizeof(object_t*),
    .tag  =OBJ_TAG,

    .layout=&TypeLayout.layout,
    .trace =trace_dtype,
    .free  =free_dtype,
    .init  =init_dtype,
    .unwrap=fixnum_unwrap,
    .wrap  =fixnum_wrap,
    .unbox =fixnum_unbox
  };

dtype_impl_t UnionTypeImpl =
  {
    .value=Ctype_object, .repr=OBJ, .data_size=sizeof(object_t*),
    .tag  =OBJ_TAG,

    .layout=&TypeLayout.layout,
    .trace =trace_utype,
    .free  =free_utype,
    .init  =init_utype,
    .unwrap=fixnum_unwrap,
    .wrap  =fixnum_wrap,
    .unbox =fixnum_unbox
  };

// data type types
type_t DataType =
  {
    { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa =isa_dtype,
    .has =none_has,
    .dtype=&DataTypeImpl
  };

type_t UnionType =
  {
    { .dtype=&DataType, .permanent=true, .hdrtag=HDR_BITS },

    .isa  =isa_dtype,
    .has  =none_has,
    .dtype=&UnionTypeImpl
  };

members_t TypeTypeMembers =
  {
    { .dtype=&MembersType, .permanent=true, .hdrtag=HDR_BITS },

    .length  =0,
    .capacity=8,
    .data    =NULL
  };

utype_impl_t TypeTypeImpl =
  {
    .members=&TypeTypeMembers.obj
  };

type_t TypeType =
  {
    { .dtype=&UnionType, .permanent=true, .hdrtag=HDR_BITS },

    .isa  =isa_utype,
    .has  =utype_has,
    .utype=&TypeTypeImpl
  };

// initialization
#define TYPE_NPUB 5ul

void init_prim_dtype( type_t *type, char *name, object_t *constructor, size_t n, map_init_t *slots )
{
  type_name( type )        = symbol( name );
  type_hash( type )        = hash_dtype( type );
  type_constructor( type ) = constructor;

  // initialize slots
  if ( n > 0 )
    {
      object_t *type_slots = dtype_slots( type );

      tb_data( type_slots ) = alloc( tb_cap( type_slots ) * sizeof(object_t*) );

      for (size_t i=0; i<n; i++)
	{
	  map_init_t slot = slots[i];

	  object_t *slot_name = symbol( slot.key.as_Cstring );

	  set_add( type_slots, (rl_value_t)slot_name );
	}
    }
}

void init_prim_utype( type_t *type, char *name, object_t *constructor, size_t n, type_t **members )
{
  type_name( type )        = symbol( name );
  type_hash( type )        = hash_utype( type_name( type ), n, members );
  type_constructor( type ) = constructor;

  object_t *umembers  = utype_members( type );

  tb_data( umembers ) = alloc( tb_cap( umembers ) *sizeof(object_t*) );

  for ( size_t i=0; i<n; i++)
    set_add( utype_members( type ), (rl_value_t)members[i] );
}

// mark and unmark functions
void rl_obj_type_mark( void )
{
  obj_mark( &NoneType.obj );
  obj_mark( &AnyType.obj );
  obj_mark( &NulType.obj );
  obj_mark( &DataType.obj );
  obj_mark( &UnionType.obj );
  obj_mark( &TypeType.obj );
  obj_mark( &SignatureType.obj );
  obj_mark( &MembersType.obj );
  obj_mark( &SlotType.obj );
  obj_mark( &SlotsType.obj );
}

void rl_obj_type_unmark( void )
{
  obj_unmark( &NoneType.obj );
  obj_unmark( &AnyType.obj );
  obj_unmark( &NulType.obj );
  obj_unmark( &DataType.obj );
  obj_unmark( &UnionType.obj );
  obj_unmark( &TypeType.obj );
  obj_unmark( &SignatureType.obj );
  obj_unmark( &MembersType.obj );
  obj_unmark( &SlotType.obj );
  obj_unmark( &SlotsType.obj );  
}

void rl_obj_type_init( void )
{
  map_init_t TypeSlots[] =
    {
      { .key = (rl_value_t)":hash",       },
      { .key = (rl_value_t)":name"        },
      { .key = (rl_value_t)":constructor" },
      { .key = (rl_value_t)":isa?",       },
      { .key = (rl_value_t)":has?",       },
    };

  map_init_t SlotSlots[] =
    {
      { .key = (rl_value_t)":hash"  },
      { .key = (rl_value_t)":name"  },
      { .key = (rl_value_t)":index" },
      { .key = (rl_value_t)":props" },
      
    };

  init_prim_dtype( &NulType, "Nul", NULL, 0, NULL );
  init_prim_dtype( &DataType, "Data", NULL, TYPE_NPUB, TypeSlots );
  init_prim_dtype( &UnionType, "Union", NULL, TYPE_NPUB, TypeSlots );
  init_prim_dtype( &SlotType, "Slot", NULL, 4, SlotSlots );
  init_prim_dtype( &SlotsType, "Slots", NULL, 0, NULL );
  init_prim_dtype( &SignatureType, "Signature", NULL, 0, NULL );
  init_prim_dtype( &MembersType, "Members", NULL, 0, NULL );

  type_t *TypeTypes[] = { &DataType, &UnionType };

  init_prim_utype( &AnyType, "Any", NULL, 0, NULL );
  init_prim_utype( &NoneType, "None", NULL, 0, NULL );
  init_prim_utype( &TypeType, "Type", NULL, 2, TypeTypes );
}
