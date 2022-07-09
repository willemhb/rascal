#include <string.h>
#include <assert.h>

#include "ctypes.h"

#include "runtime/object.h"
#include "runtime/memory.h"
#include "describe/utils.h"


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

// constructors ---------------------------------------------------------------
value_t new_cons( int n ) {
  assert( n >= 1 );

  cons_t *out = allocate( n * sizeof( cons_t ), false );
  cons_t *buf = out;

  for ( int i=0; i<n; i++ ) cdr( out ) = (value_t)(buf++);

  return tagp( out, tag_cons );
}

// accessors ------------------------------------------------------------------
array_ref(vector, value_t)
array_set(vector, value_t)
