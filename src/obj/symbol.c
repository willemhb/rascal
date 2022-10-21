#include <string.h>
#include <stdio.h>

#include "obj/symbol.h"
#include "obj/object.h"
#include "utils/hashing.h"
#include "obj/cvalue.h"
#include "vm/memory.h"

#define gsfmt "%s#%lu"
#define gs_inline_size 21

typedef struct
{
  hash_t  hash;
  char   *name;
  idno_t  idno;
  bool    gensym;
  bool    keyword;
} symbol_init_t;

// symbol implementation
void init_symbol( object_t *self, type_t *type, size_t n, void *data )
{
  symbol_init_t *ini = data;
  ini->idno          = ++symtype_impl( type )->counter;
  ini->keyword       = ini->name[0] == ':';

  if ( ini->gensym )
    {
      char buffer[n+gs_inline_size];
      snprintf( buffer, n+gs_inline_size, gsfmt, ini->name, ini->idno );
      ini->hash = hash_string( buffer );
    }

  sym_name( self )    = duplicate_bytes( ini->name, n );
  sym_hash( self )    = ini->hash;
  sym_idno( self )    = ini->idno;
  sym_gensym( self )  = ini->gensym;
  sym_keyword( self ) = ini->keyword;
}

// symbol table implementation
int cmp_symbols_keys( rl_value_t x, rl_value_t y )
{
  return strcmp( x.as_Cstring, y.as_Cstring );
}

hash_t hash_symbols_key( rl_value_t key )
{
  return hash_string( key.as_Cstring );
}

size_t symbols_key_size( rl_value_t key )
{
  return strlen( key.as_Cstring ) + 1;
}

rl_value_t symbols_get_key( object_t *obj )
{
  return (rl_value_t) { .as_Cstring=((symbol_t*)obj)->name };
}

rl_value_t symbols_get_value( object_t *obj )
{
  return (rl_value_t) { .as_object=obj };
}

object_t *symbol_intern( object_t *obj, rl_value_t key, hash_t h, size_t j )
{
  symbol_init_t ini = { .name=key.as_Cstring, .hash=h, .gensym=false };
  return ( tb_data( obj )[j] = new_obj( &SymbolType, strlen( ini.name )+1, &ini ) );
}

object_t *symbol( char *name )
{
  rl_value_t out;
  table_intern( &Symbols.obj, &out, (rl_value_t)name );
  return out.as_object;
}

object_t *gensym( char *name )
{
  if ( name == NULL )
    name = "symbol";

  symbol_init_t ini = { .name=name, .gensym=true };
  return new_obj( &SymbolType, strlen(name)+1, &ini );
}

// globals
static const size_t SymbolsMinC = 512;

// global symbol table
symbols_t Symbols =
  {
    .obj     ={ .dtype=&SymbolsType, .permanent=true, .hdrtag=HDR_BITS },
    .length  =0,
    .capacity=SymbolsMinC,
    .data    =NULL
  };

// metaobjects
// symbol_impl_t
void trace_symbol_impl( object_t *obj )
{
  obj_mark( (object_t*)((symbol_impl_t*)obj)->table );
}

type_t *SymbolImplSigData[] =
  {
    &UInt64Type, &SymbolsType
  };

signature_t SymbolImplSig  =
  {
    .obj     ={ .dtype=&SignatureType, .hdrtag=HDR_BITS },
    .length  =2,
    .capacity=2,
    .data    =SymbolImplSigData
  };

layout8_t SymbolImplLayout =
  {
    { .base_size=sizeof(symbol_impl_t), .n_fields=2 },
    {
      { sizeof(idno_t),     NTUINT, false, offsetof( symbol_impl_t, counter ) },
      { sizeof(symbols_t*), NTPTR,  false, offsetof( symbol_impl_t, table )   }
    }
  };

dtype_impl_t SymbolImplImpl =
  {
    .value    =Ctype_object,
    .repr     =OBJ,
    .data_size=sizeof(symbol_impl_t*),
    .tag      =OBJ_TAG,
    .layout   =&SymbolImplLayout.layout,
    .signature=&SymbolImplSig.obj,
    .trace    =trace_symbol_impl,
    .unwrap   =fixnum_unwrap,
    .wrap     =fixnum_wrap,
    .unbox    =fixnum_unbox,
  };

type_t SymbolImplType =
  {
    .obj = { .dtype=&DataType, .hdrtag=HDR_BITS },

    .isa  = isa_dtype,
    .has  = none_has,
    .dtype=&SymbolImplImpl 
  };

// symbol_t
type_t *SymbolSigData[] =
  {
    &UInt64Type,
    &CStringU8Type,
    &FixnumType,
    &BooleanType,
    &BooleanType
  };

signature_t SymbolSig =
  {
    .obj     = { .dtype=&SignatureType, .hdrtag=HDR_BITS },
    .length  =5,
    .capacity=5,
    .data    =SymbolSigData
  };

layout8_t SymbolLayout =
  {
    { .base_size=sizeof(symbol_t), .n_fields=5 },
    {
      { sizeof(hash_t), NTUINT, false, offsetof( symbol_t, hash )    },
      { sizeof(char*),  NTPTR,  false, offsetof( symbol_t, name )    },
      { sizeof(idno_t), NTUINT, false, offsetof( symbol_t, idno )    },
      { sizeof(bool),   NTSINT, false, offsetof( symbol_t, gensym )  },
      { sizeof(bool),   NTSINT, false, offsetof( symbol_t, keyword ) }
    }
  };

symbol_impl_t SymbolImpl =
  {
    .obj    ={ .dtype=&SymbolImplType, .permanent=true, .hdrtag=HDR_BITS },
    .counter=0,
    .table  =&Symbols,
  };

tb_impl_t SymbolsMethods =
  {
    { .dtype=&TableImplType, .permanent=true, .hdrtag=HDR_BITS },

    .minc=SymbolsMinC,
    .entry_type=&SymbolType,
    .pad=pad_table_size,
    .keysize=symbols_key_size,
    .keyhash=hash_symbols_key,
    .keycmp=cmp_symbols_keys,
    .getkey=symbols_get_key,
    .getvalue=symbols_get_value,
    .setvalue=NULL
  };

dtype_impl_t SymbolsImpl =
  {
    .value=Ctype_object, .repr=OBJ, .data_size=sizeof(object_t*),
    
    .layout=&TableLayout.layout,

    .init  =init_set,
    .trace =trace_table,
    .free  =free_table,
    .new   =NULL,

    .impl  =&SymbolsMethods.obj
  };

type_t SymbolsType =
  {
    { .dtype=&DataType },

    .dtype =&SymbolsImpl
  };

// symbol table implementation
void rl_obj_symbol_init( void )
{
  init_set( &Symbols.obj, &SymbolType, 0, NULL );
}

void rl_obj_symbol_mark( void )
{
  obj_mark( &SymbolsType.obj );
  obj_mark( &SymbolType.obj );
}
