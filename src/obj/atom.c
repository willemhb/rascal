#include <string.h>

#include "obj/atom.h"
#include "obj/str.h"
#include "obj/table.h"
#include "obj/cons.h"

#include "mem.h"

static const arity_t symt_min_cap = 256;
static const char *gsfmt    = "%s#%zu";
static const char *fallback = "symbol";

// api
atom_t *new_atom( char *name, hash_t h, bool hp, bool gensym )
{
  hash_t self_hash;
  
  if (name == NULL)
    {
      assert(gensym);
      name = (char*)fallback;
    }

  if (!hp)
    {
      h = hash_string(name);
    }

  self_hash = h;
  self_hash = mix_hashes( Symbols.counter, self_hash );
  self_hash = mix_hashes( atom_type, self_hash );
  
  arity_t n  = strlen(name);
  flags_t fl = 0 | atom_fl_gensym*gensym | atom_fl_keyword*(name[0]==':');

  str_t *name_str = new_str( enc_ascii, n, name );

  atom_t *out = alloc(sizeof(atom_t));

  init_obj((obj_t*)out, atom_type, fl );

  out->name = name_str;
  out->hash = self_hash;
  out->idno = Symbols.counter++;

  return out;
}

val_t intern( char *name )
{
  val_t string = tag_val((void*)name, POINTER);
  kv_t *buffer;

  table_put( &Symbols.table, string, (obj_t**)&buffer );

  return kv_val(buffer);
}

bool is_keyword( val_t val )
{
  return is_atom(val)
    && flag_p(as_obj(val)->flags, atom_fl_keyword );
}

bool is_gensym( val_t val )
{
  return is_atom(val)
    && flag_p(as_obj(val)->flags, atom_fl_gensym );
}

// initialization
void atom_init( void )
{
  // initialize the symbol table
  Symbols.counter = 1;
  init_table(&Symbols.table, symt_min_cap, table_fl_symt );
}
