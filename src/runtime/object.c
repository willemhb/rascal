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

  if ( t < tag_mapping )
    return t;

  if ( t < tag_immediate )
    return obtype( x );

  return wtag( x );
}

size_t val_sizeof( value_t x ) {
  type_t t = val_typeof( x );
  size_t base = TypeSizes[t];

  if ( t == type_cons || t == type_closure || ( (t&7) == tag_immediate ) )
    return base;

  if ( Collecting || asuptr( x ) + base == obdata( x ) )
    return base + obsize( x );

  return base;
}

hash_t val_hash( value_t x ) {
  type_t t = val_typeof( x );

  if ( flagp( t, tag_immediate ) ) return int64hash( x );
  if ( t == type_symbol ) return shash( x );
  if ( t == type_string ) return strhash( sdata( x ) );
  return pointerhash( asptr( x ) );
}

static int list_order( value_t x, value_t y ) {
  int o = 0;

  while ( consp( x ) && consp( y ) ) {
    o = val_order( car( x ), car( y ) );

    if ( o )
      return o;

    x = cdr( x );
    y = cdr( y ); 
  }

  if ( consp( y ) ) return -1;
  if ( consp( x ) ) return  1;
  if ( nullp( x ) ) return -1;
  if ( nullp( y ) ) return 1;

  return val_order( x, y );
}

static int tuple_order( tuple_t *x, tuple_t *y ) {
  int o = 0;
  size_t n = min( x->len, y->len );

  for (size_t i=0; i<n; i++)
    if ((o=val_order( x->data[i], y->data[i] )))
      return o;

  if ( x->len < y->len )
    return -1;
  if ( x->len > y->len )
    return  1;
  return 0;  
}

static int mem_order( binary_t *x, binary_t *y ) {
  size_t n = min( (size_t)x->size, (size_t)y->size );
  int    o = memcmp( x->data, y->data, n );
  if ( o )
    return o;
  if ( x->len < y->len )
    return -1;
  if ( x->len > y->len )
    return 1;
  return 0;
}

static int table_order( root_t *x, root_t *y ) {
  int node_order( node_t *x, node_t *y ) {
    if ( !x )
      return -(!!y);
    if ( !y )
      return 1;

    int o;

    if ( (o = node_order( x->left, y->left )) )
      return o;
    
    if ( (o = strcmp( x->base.data, y->base.data )) )
      return o;

    return node_order( x->right, y->right ); 
  }

  return node_order( x->data, y->data );
}

int val_order( value_t x, value_t y ) {
  type_t xt = val_typeof( x ), yt = val_typeof( y );

  if ( xt != yt )
    return ord_uint( xt, yt );

  switch ( xt ) {
  case type_cons: case type_null:
    return list_order( x, y );

  case type_symbol: case type_string:
    return strcmp( (char*)obdata( x ), (char*)obdata( y ) );

  case type_bytecode:
    return mem_order( asptr( x ), asptr( y ) );

  case type_port:
    return ord_long( fileno( pios( x ) ), fileno( pios( y ) ) );

  case type_vector:
    return tuple_order( asptr( x ), asptr( y ) );

  case type_table:
    return table_order( asptr( x ), asptr( y ) );

  default:
    return ord_int( asint( x ), asint( y ) );
  }
}

inline char *val_typename( value_t x ) {
  return TypeNames[val_typeof( x )];
}

// predicates -----------------------------------------------------------------
mk_tag_p(cons)
mk_tag_p(symbol)
mk_tag_p(closure)
mk_tag_p(port)
mk_tag_p(tuple)
mk_tag_p(table)
mk_tag_p(binary)
mk_tag_p(immediate)

mk_type_p(string)
mk_type_p(bytecode)
mk_type_p(integer)
mk_type_p(character)
mk_type_p(builtin)
mk_type_p(type)

inline bool listp( value_t x ) { return consp( x ) || nullp( x ); }

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
array_put( vector, value_t, false )

array_ref( string, char )
array_set( string, char ) 
array_resize( string, char, true )
array_put( string, char, true )

array_ref( bytecode, ushort )
array_set( bytecode, ushort )
array_resize( bytecode, ushort, false )
array_put( bytecode, ushort, false )

// mapping types --------------------------------------------------------------
