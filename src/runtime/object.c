#include <string.h>
#include <assert.h>

#include "ctypes.h"
#include "numutils.h"
#include "hashing.h"

#include "runtime/object.h"
#include "runtime/memory.h"
#include "describe/utils.h"
#include "describe/sequence.h"

// utilities ------------------------------------------------------------------
type_t val_typeof( value_t x ) {
  int t = vtag( x );
  switch ( t ) {
  case tag_cons:
    return x ? type_cons : type_null;

  case tag_symbol ... tag_closure:
    return t;

  case tag_vector ... tag_binary:
    return obtype( x );

  default:
    return wtag( x );
  }
}

size_t val_sizeof( value_t x ) {
  type_t t = val_typeof( x );

  if ( Sizeof[t] ) return Sizeof[t](x);
  return TypeSizes[t];
}

size_t sym_sizeof( value_t x ) {
  return sizeof(symbol_t) + strlen( symname( x ) );
}

size_t arr_sizeof( value_t x ) {
  if ( vdata( x ) != (((value_t*)asptr(x)) + 4) )
    return 32;

  type_t t = val_typeof( x );
  return Ctype_size( TypeEltypes[t] ) * vlen( x ) + TypeSizes[t];
}

size_t port_sizeof( value_t x ) {
  return sizeof(port_t) + strlen( portname( x ) );
}

// predicates -----------------------------------------------------------------
mk_tag_p(cons)
mk_tag_p(symbol)
mk_tag_p(closure)
mk_tag_p(port)
mk_tag_p(vector)
mk_tag_p(table)
mk_tag_p(binary)
mk_tag_p(immediate)

mk_type_p(string)
mk_type_p(bytecode)
mk_type_p(dict)
mk_type_p(integer)
mk_type_p(character)
mk_type_p(byte)
mk_type_p(builtin)
mk_type_p(form)
mk_type_p(opcode)
mk_type_p(type)

inline bool listp( value_t x ) { return vtag(x) == tag_cons; }

// constructors ---------------------------------------------------------------
value_t new_cons( int n ) {
  assert( n >= 1 );

  cons_t *out = allocate( n * sizeof( cons_t ), false );
  cons_t *buf = out;

  for ( int i=0; i<n; i++ ) cdr( out ) = (value_t)(buf++);

  return tagp( out, tag_cons );
}

int cons_len( value_t x ) {
  int l = 0; value_t v;

  for_cons( &x, v ) l++;

  return l;
}

value_t cons_nrev( value_t x ) {
  value_t tmp, prev = rnull;

  while ( consp( x ) ) {
    tmp      = cdr( x );
    cdr( x ) = prev;
    prev     = x;
    x        = tmp;
  }

  return prev;
}

value_t cons_ncat( value_t x, value_t y ) {
  if ( nullp( x ) )
    return y;

  value_t curr = x, prev = rnull;

  while ( consp( curr ) ) {
    prev = curr;
    curr = cdr( curr );
  }

  cdr( prev ) = y;
  return x;
}

// arrays ---------------------------------------------------------------------
array_ref( vector, value_t )
array_set( vector, value_t )
array_resize( vector, value_t, false )
array_put( vector, value_t )

array_ref( string, char )
array_set( string, char ) 
array_resize( string, char, true )
array_put( string, char )

array_ref( bytecode, ushort )
array_set( bytecode, ushort )
array_resize( bytecode, ushort, false )
array_put( bytecode, ushort )

// mapping types --------------------------------------------------------------
  int dict_locate( value_t d, value_t k, node_t **buf, hash_t *hb ) {
  node_t *n = mdata( d ); int o = 0;
  hash_t  h = val_hash( k );

  *buf = n;

  while ( n ) {
    o = ord_int( h, mhash( n ) );
    if ( !o )
      o = val_order( k, mval( n ) );
  }
}
