#include <string.h>
#include <stdio.h>

#include "obj/symbol.h"
#include "obj/object.h"
#include "utils/hashing.h"


#define gsfmt "%s#%lu"
#define gs_inline_size 21

typedef struct
{
  char   *name;
  hash_t  hash;
  bool    gensym;
} symbol_init_t;

// symbol implementation
void init_symbol( object_t *self, type_t *type, size_t n, void *data )
{
  symbol_init_t *ini = data;

  
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

// globals
static const size_t SymbolsMinC = 64;

symbols_t Symbols;

tb_impl_t SymbolsMethods =
  {
    { .dtype=&TableImplType, .permanent=true, .hdrtag=HDR_BITS },

    .minc=SymbolsMinC,
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
  init_dtype( &SymbolType, "symbol", );
}

void rl_obj_symbol_mark( void )
{
  obj_mark( &SymbolsType.obj );
  obj_mark( &SymbolType.obj );
}
