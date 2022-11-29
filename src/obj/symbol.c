#include <string.h>

#include "obj/symbol.h"
#include "obj/type.h"

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
void trace_symbol(object_t *object);
void free_symbol(object_t *object);

struct vtable_t SymbolMethods =
  {
    .init=init_symbol,
    .trace=trace_symbol,
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
void init_symbol(object_t *object)
{
  symbol_t *symbol = (symbol_t*)object;

  symbol->idno = ++SymbolCounter;
}

void trace_symbol(object_t *object)
{
  symbol_t *symbol = (symbol_t*)object;

  mark_value(symbol->bind);
}

void free_symbol(object_t *object)
{
  symbol_t *symbol = (symbol_t*)object;

  free_string(symbol->name);
}

symbol_t *make_symbol( char *name )
{
  symbol_t *symbol = (symbol_t*)make_object(&SymbolType);

  symbol->name = make_string(strlen8(name), name);
  symbol->hash = hash_str8(name);
  symbol->bind = NUL;

  return symbol;
}

/* runtime */

void rl_obj_symbol_init( void )
{
  /* initialize type */
  init_object(&SymbolType.type.obj);
}

void rl_obj_symbol_mark( void )
{
  
}

/* convenience */
