#include <string.h>

#include "hashing.h"
#include "memutils.h"

#include "symbol.h"
#include "array.h"
#include "function.h"
#include "object.h"
#include "runtime.h"


static value_t intern( symbols_t **table, char *name ) {
  hash_t h = strhash( name );

  while (*table) {
    int o = ord_ulong( h, (*table)->base.hash );

    if (o == 0)
      o = strcmp( name, (*table)->base.name );

    if (o < 0)
      table = &(*table)->left;

    else if (o > 0)
      table = &(*table)->right;

    else
      break;
  }

  if (*table == NULL) {
    size_t l = strlen(name);
    symbols_t* loc = malloc_s( sizeof(symbols_t) + l + 1);
    symbol_t* new = &(loc->base);
    init_symbol( new, h, name, l );
    *table = loc;
  }

  return tagp( &(*table)->base, tag_boxed );
}

value_t symbol( char *name ) {
  return intern( &Symbols, name );
}


value_t gensym( char *name ) {
  gensym_s( name );
  return pop();
}

index_t gensym_s( char *name ) {
  static const char *gs_fmt = "%s#%lu";

  if (name == NULL)
    name = "symbol";

  size_t l = strlen(name);
  char namebuf[l+1], hashbuf[l+2+SAFE_NUMBER_BUFFER_SIZE];
  strcpy(namebuf, name);
  snprintf( hashbuf, l+2+SAFE_NUMBER_BUFFER_SIZE, gs_fmt, namebuf, Symcnt+1 );
  hash_t h = strhash( hashbuf );
  symbol_t *out = allocate( sizeof(symbol_t) + l + 1);
  init_symbol( out, h, namebuf, l );
  value_t sym = tagp( out, tag_boxed );
  push_s( "symbol", sym );
  return Sp;
}


// predicates -----------------------------------------------------------------
mk_type_p(symbol)

bool is_gensym( value_t x ) { return is_symbol(x) && !is_managed(x); }
bool is_keyword( value_t x ) { return is_symbol(x) && sname(x)[0] == ':'; }
bool is_bound( value_t x ) { return is_symbol(x) && sbind(x) != val_unbound; }

// safecasts ------------------------------------------------------------------
mk_safe_cast(symbol, symbol_t*, pval, true)

// methods --------------------------------------------------------------------
int symbol_order( value_t x, value_t y ) {
  int o;
  
  if ((o = strcmp( assymbol(x)->name, assymbol(y)->name )))
    return o;
  
  return ord_ulong( assymbol(x)->idno, assymbol(y)->idno );
}


size_t symbol_prin( FILE *ios, value_t x ) {
  if (is_gensym(x))
    return fprintf( ios, "%s#%ld", sname(x), sidno(x) );

  return fprintf( ios, "%s", sname(x) );
}

// builtins ------------------------------------------------------------------
r_predicate(symbol)
r_predicate(gensym)
r_predicate(keyword)
r_predicate(bound)

void r_builtin(symbol) {
  n = oargc( "symbol", n, 3, 0, 1, 2 );

  if (n == 0) {
    gensym_s(NULL);
    return;
  }

  bool interned = true;

  if (n == 2) {
   require( "symbol",
	     is_keyword(Sref(1)),
	     "# wanted a :symbol()" );

    require( "symbol",
	     Sref(1) == r_kw_generate || Sref(1) == r_kw_intern,
	     "unknown option '%s'",
	     sname(Sref(1)) );

    interned = Sref(1) == r_kw_intern;
    pop();
  }

  type_t arg1t = oargt( "symbol", Sref(1), 2, type_symbol, type_string );

  if (arg1t == type_symbol && interned)
    return;
  
  char *name = is_symbol(Sref(1)) ? sname(Sref(1)) : adata(Sref(1));

  pop();

  if (interned)
    symbol_s(name);

  else
    gensym_s(name);
}

// initialization -------------------------------------------------------------
void symbol_init( void ) {
  r_kw_generate = symbol( ":generate" );
  r_kw_intern = symbol( ":intern" );
  
  Typenames[type_symbol] = "symbol";
  
  prin_dispatch[type_symbol] = symbol_prin;
  order_dispatch[type_symbol] = symbol_order;

  builtin("symbol", builtin_symbol );
  builtin("symbol?", builtin_is_symbol );
  builtin("keyword?", builtin_is_keyword );
  builtin("gensym?", builtin_is_gensym );
  builtin("bound?", builtin_is_bound );
}
