#include <string.h>

#include "obj/symbol.h"
#include "obj/type.h"

#include "vm/obj/vm.h"
#include "vm/obj/support/string.h"
#include "vm/obj/support/symbol_table.h"

#include "util/hashing.h"
#include "util/string.h"

/* commentary

   Symbol type and symbol table. */

/* instantiations */

/* C types */

/* globals */
void init_symbol(object_t *object);
void free_symbol(object_t *object);

struct vtable_t SymbolMethods =
  {
    .init=init_symbol,
    .trace=NULL,
    .free=free_symbol
  };

struct layout_t SymbolLayout =
  {
    .vmtype=vmtype_objptr,
    .obsize=sizeof(symbol_t)
  };

struct datatype_t SymbolType =
  {
    {
      .obj = obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name="symbol"
    },

    .methods=&SymbolMethods,
    .layout=&SymbolLayout
  };

/* API */
/* object runtime methods */
void init_symbol(object_t *object)
{
  symbol_t *symbol = (symbol_t*)object;

  symbol->idno = ++SymbolCounter;
}

void free_symbol(object_t *object)
{
  symbol_t *symbol = (symbol_t*)object;

  free_string(symbol->name);
}

/* constructors */
symbol_t *make_symbol( const char *name )
{
  symbol_t *symbol = (symbol_t*)make_object(&SymbolType);

  symbol->name = make_string(strlen8(name), (char*)name);
  symbol->hash = hash_str8(name);

  return symbol;
}

value_t symbol( const char *name )
{
  symbol_t *interned = intern_string((char*)name);
  return tag_object(interned);
}

/* accessors */
#include "tpl/impl/record.h"

GET(symbol, name, string_t);
GET(symbol, hash, ulong);
GET(symbol, idno, ulong);

/* runtime dispatch */
void rl_obj_symbol_init( void )
{
  /* initialize type */
  gl_init_type(SymbolType);
}

void rl_obj_symbol_mark( void )
{
  gl_mark_type(SymbolType);
}

void rl_obj_symbol_cleanup( void )
{
  gl_free_type(SymbolType);
}

/* convenience */
/* global binding */
void define( char *name, value_t value )
{
  symbol_t *interned = intern_string(name);
  define_at_toplevel( interned, value );
}
