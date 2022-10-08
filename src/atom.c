#include "atom.h"
#include "memory.h"
#include "utils.h"
#include "port.h"

// symbol & symbol table
OBJ_NEW(atom);

void free_atom(object_t *obj)
{
  atom_t *atom = (atom_t*)obj;
  dealloc_vec(atom->name, strlen(atom->name), char );
}

void init_atom(atom_t *atom, symt_t *symt, char *name )
{
  init_obj(&atom->obj, ATOM, (name[0] == ':') * atom_fl_keyword );

  atom->name = duplicate(name);
  atom->idno = symt->idno++;
}

void prin_atom(stream_t *port, value_t value)
{
  port_prinf( port, as_atom(val)->name );
}

TABLE_INIT(symt, atom, SYMT);
TABLE_FREE(symt, atom);
TABLE_MARK(symt, atom);
TABLE_CLEAR(symt, atom);
TABLE_REHASH(symt, atom); 
TABLE_RESIZE(symt, atom, SYMT);
TABLE_PUT(symt, atom, char*, name, hash_string, strcmp);

atom_t *symt_intern(symt_t *symt, char *name)
{
  atom_t *buf;

  if (symt_put(symt, name, &buf))
      init_atom(buf, symt, name);

  return buf;
}

value_t symbol( char *name )
{
  atom_t *atom = symt_intern(&Symbols, name );

  return tag_ptr(atom, OBJ);
}

// initialization
static void init_famous_symbols( void )
{
  Quote = symbol("quote");
  Error = symbol("error");
  

  obj_flags(Quote) |= atom_fl_keyword;
  
}

void atom_init( void )
{
  init_symt(&Symbols);
  init_famous_symbols();
};
