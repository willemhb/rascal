#include <string.h>

#include "numutils.h"

#include "runtime/table.h"
#include "runtime/object.h"

value_t intern( root_t *t, char *s, int n, hash_t h, value_t (*do_intern)( char *s, int n, hash_t h ) ) {
  node_t **c = &mdata( t );
  int o = 0;

  while ( *c ) {
    o = ord_uint( h, mhash( *c ) );

    if (!o)
      o = strcmp( s, mkey( *c ) );
    if ( o < 0 )
      c = &mright( *c );
    else if ( o > 0 )
      c = &mleft( *c );
    else
      break;
  }

  if ( !o ) {
    *c = new_node();
    mbind( *c ) = do_intern( s, n, h );
  }

  return mbind( *c );
}

value_t do_intern_symbol( node_t *b, char *s, int n, hash_t h );
value_t do_intern_character( node_t *b, char *s, int n, hash_t h );
