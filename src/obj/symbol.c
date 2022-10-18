#include <string.h>
#include <stdio.h>

#include "symbol.h"
#include "memory.h"
#include "utils.h"

#define gsfmt "%s#%lu"
#define gs_inline_size 21

extern symbol_t *Symbols;

symbol_t **find_symbol(char *name, hash_t hash)
{ 
  symbol_t **buf = &Symbols;

  while (*buf)
    {
      int o = cmp(hash, (*buf)->hash) ? : strcmp(name, (*buf)->name);

      if (o < 0)
	buf = &(*buf)->left;

      else if (o > 0)
	buf = &(*buf)->right;

      else
	break;
    }

  return buf;
}

symbol_t *intern_symbol(char *name)
{
  assert(name != NULL);
  assert(*name != '\0');

  hash_t hash = mix_hashes( SYMBOL, hash_string(name) );

  symbol_t **buf = find_symbol( name, hash );

  if (*buf == NULL)
      *buf = mk_symbol( name, hash, false );

  return *buf;
}

symbol_t *new_symbol( size_t n )
{
  return alloc(sizeof(symbol_t) + n + 1);
}

size_t symbol_size(object_t *object)
{
  return sym_length(object) + 1;
}

hash_t symbol_hash( object_t *object )
{
  return sym_hash( object );
}

ord_t symbol_order( object_t *x, object_t *y )
{
  ord_t o = strcmp( sym_name(x), sym_name(y) );

  if (o)
    return o;

  return cmp( sym_idno(x), sym_idno(y) );
}

symbol_t *mk_symbol(char *name, hash_t hash, bool isGensym)
{
  static idno_t counter = 0;

  idno_t  idno   = ++counter;

  if (name == NULL)
    {
      assert(isGensym);
      name = "symbol";
    }

  assert(*name != '\0');

  arity_t length = strlen(name);

  if (isGensym)
    {
      size_t bufsz = length+gs_inline_size+1;
      char namebuffer[bufsz];

      snprintf(namebuffer, bufsz, gsfmt, name, idno );
      hash = mix_hashes( SYMBOL, hash_string(namebuffer) );
    }

  bool isKeyword = name[0] == ':';
  
  symbol_t *out = new_symbol(length+1);

  init_symbol(out, name, hash, idno, isGensym, isKeyword, length);

  return out;
}

void init_symbol( symbol_t *symbol, char *name, hash_t hash, idno_t idno, bool isGensym, bool isKeyword, arity_t length )
{
  init_object(&symbol->object, SYMBOL, false);

  if (!isGensym)
    symbol->object.gcbits = BLACK;
  
  strcpy( symbol->name, name );

  symbol->hash      = hash;
  symbol->idno      = idno;
  symbol->isGensym  = isGensym;
  symbol->isKeyword = isKeyword;
  symbol->length    = length;
  symbol->left      = symbol->right = NULL;
}

value_t symbol( char *name )
{
  symbol_t *out = intern_symbol(name);
  return tag(out, SYMBOL);
}

value_t gensym( char *name )
{
  symbol_t *out = mk_symbol( name, 0, true );
  return tag(out, SYMBOL);
}

void symbol_init( void )
{
  Symbols = NULL;
}
