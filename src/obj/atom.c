#include <string.h>

#include "obj/atom.h"
#include "obj/str.h"

#include "mem.h"
#include "template/table.h"

static const arity_t symt_min_cap = 8;
static const double  symt_load = 0.75;

DECL_TABLE(symt, str_t*, atom_t*);

symt_t SymbolTableObject;

static bool sym_table_cmp( str_t *interning, str_t *interned )
{
  return strcmp(interning->vals, interned->vals) == 0;
}

static hash_t sym_table_hash( str_t *interning )
{
  return interning->hash;
}

// generic implementation
DECL_TABLE_API(symt, str_t*, atom_t*);

TABLE_INIT(symt)
TABLE_TRACE(symt)
TABLE_FINALIZE(symt)
TABLE_REHASH(symt)
TABLE_RESIZE(symt, str_t*, atom_t*)     
TABLE_GET(sym_table, str_t*, atom_t*)
TABLE_PUT(sym_table, str_t*, atom_t*)
TABLE_POP(sym_table, str_t*, atom_t*)


// interning wrappers ---------------------------------------------------------
obj_t *obj_atom( obj_t *obj )
{
  assert( obj->type == str_type );
  return str_atom( (str_t*)obj );
}

obj_t *char_atom( char *name )
{
  str_t buffer;
  str_ini_t ini = { name, true, hash_string(name) };
  arity_t   len = strlen( name );

  initialize((obj_t*)&buffer, str_type, len, &ini, STATIC_OBJ|STATIC_DATA );
  return str_atom( &buffer );
}

obj_t *str_atom( str_t *str )
{
  sym_table_kv_t *buffer;

  if ( sym_table_put( (obj_t*)SymbolTable, str, (obj_t**)&buffer ) )
    {
      if (obj_is_static((obj_t*)str))
	str = (str_t*)dup( (obj_t*)str );

      buffer->val = (atom_t*)construct( atom_type, 1, str, 0 );
    }

  return (obj_t*)buffer->val;
}

// memory methods -------------------------------------------------------------
void init_atom( obj_t *obj, arity_t i, void *data, flags_t fl )
{
  (void)fl;
  (void)i;
  
  atom_t *atom = (atom_t*)obj;
  atom->idno   = ++SymbolCount;
  atom->name   = data;
  atom->hash   = mix_hashes( atom_type, atom->name->hash );
}

// toplevel dispatch ----------------------------------------------------------
rt_table_t AtomRtTable, SymTableRtTable, SymTableKvRtTable;

static void atom_init_globals( void )
{
  SymbolCount = 0;
  SymbolTable = &SymbolTableObject;
  initialize( (obj_t*)SymbolTable, sym_table_type, 0, NULL, STATIC_OBJ );
}

static void atom_init_types( void )
{
  
}

void atom_init( void )
{
  atom_init_globals();
  atom_init_types();
}

void atom_mark( void )
{
  mark_obj( (obj_t*)SymbolTable );
}
