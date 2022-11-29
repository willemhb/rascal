#include <string.h>

#include "obj/symbol.h"

#include "vm/obj/support/symbol_table.h"

#include "tpl/impl/hashmap.h"

/* commentary */

/* instantiations */
#include "tpl/impl/hashmap.h"

bool symbol_table_cmp( const char* x, const char* y )
{
  return strcmp(x, y) == 0;
}

HASHMAP(symbol_table, char*, symbol_t*, pad_table_size, hash_str8, symbol_table_cmp, NULL, NULL);

/* C types */

/* globals */
symbol_table_t SymbolTable = { .len=0, .cap=0, .table=NULL };

ulong SymbolCounter = 0;

/* API */
symbol_t *intern_string( char *name )
{
  void **spc = symbol_table_locate(&SymbolTable, name);
  symbol_t *out;

  if ( hmap_key(spc, 0, char*) == NULL )
    {
      out = make_symbol(name);

      hmap_key(spc, 0, char*)     = out->name;
      hmap_val(spc, 0, symbol_t*) = out;

      resize_symbol_table(&SymbolTable, SymbolTable.len+1);
    }

  else
    out = hmap_val(spc, 0, symbol_t*);

  return out;
}

void mark_symbol_table(symbol_table_t *symbol_table)
{
  /* don't traverse remaining empty table cells once all objects have been marked */
  for ( size_t i=0, n=0; i < symbol_table->cap && n < symbol_table->len; i++ )
    {
      object_t *object = hmap_val(symbol_table, i, object_t*);

      if (object == NULL)
	continue;

      mark_object(object);
      n++;
    }
}

/* runtime */
#define N_SYMBOLS 256

void rl_vm_obj_support_symbol_table_init( void )
{
  /* initialize symbol table & counter */
  init_hmap(&SymbolTable, N_SYMBOLS, pad_table_size);
  init_symbol_table(&SymbolTable);
  SymbolCounter = 0;  
}

void rl_vm_obj_support_symbol_table_mark( void )
{
  mark_symbol_table(&SymbolTable);
}
