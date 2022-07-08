#include "object/table.h"
#include "object/tuple.h"
#include "object/pair.h"
#include "numutils.h"
#include "hashing.h"

bool_t  tabhas(object_t *tab, value_t key) {
  object_t *keys  = tabkeys( tab ),
           *binds = tabvals( tab );

  value_t *vals   = tupledata( binds );

  hash_t h = rhash( key );

  while (keys) {
    index_t loc = tabloc( keys );
    value_t entry = vals[loc];
    int_t o = ord_ulong( h , phash( entry ) );

    if (o == 0)
      o = order( key, car( entry ) );
    if (o == 0)
      return true;
    if (o < 0)
      keys = tableft( keys );
    else
      keys = tabright( keys );
  }
  return false;
}


value_t tabref(object_t *tab, value_t key) {
  object_t *keys  = tabkeys( tab ),
           *binds = tabvals( tab );

  value_t *vals   = tupledata( binds );
  hash_t h        = rhash( key );

  while (keys) {
    value_t entry = vals[tabloc( keys )];
    int_t o = ord_ulong( h , phash( entry ) );

    if (o == 0)
      o = order( key, car( entry ) );
    if (o == 0)
      return cdr( entry );
    else if (o < 0)
      keys = tableft( keys );
    else
      keys = tabright( keys );
  }
  return rnone;
}

value_t tabset(object_t *tab, value_t key, value_t bind );
value_t tabput(object_t *tab, value_t key, value_t bind );
value_t tabpop(object_t *tab, value_t key);

value_t intern(object_t *t, char_t *k, index_t i, index_t (*constructor)( char_t *k, hash_t h ) ) {
  hash_t h = strhash( k );

  object_t *keys = tabkeys( t ), *prev = t;
  value_t  *vals = tupledata( tabvals( t ) );

  while (keys) {
    hash_t kh = phash( tabloc( keys ) );
    int_t  o  = ord_ulong( );
  }
}

value_t mk_table( value_t *args, size_t nargs, bool_t global );
value_t mk_dict( value_t *args, size_t nargs, bool_t global );
value_t mk_set( value_t *args, size_t nargs, bool_t global );
