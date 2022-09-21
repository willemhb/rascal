#include <string.h>

#include "obj/atom.h"
#include "obj/str.h"

#include "mem.h"
#include "template/table.h"

static const arity_t sym_table_min_cap = 8;
static const double  sym_table_load = 0.75;


static bool sym_table_cmp( str_t *interning, str_t *interned )
{
  return strcmp(interning->vals, interned->vals) == 0;
}

static hash_t sym_table_hash( str_t *interning )
{
  return interning->hash;
}

// generic implementation
TABLE(sym_table, str_t*, atom_t*);

TABLE_INIT(sym_table)
TABLE_TRACE(sym_table)
TABLE_FINALIZE(sym_table)
TABLE_REHASH(sym_table)
TABLE_RESIZE(sym_table, str_t*, atom_t*)     
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

// hashing functions ----------------------------------------------------------


// toplevel dispatch ----------------------------------------------------------
void atom_init( void )
{
  
}
