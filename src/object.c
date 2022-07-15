#include <string.h>

#include "numutils.h"
#include "memutils.h"

#include "object.h"
#include "memory.h"

bool numberp( value_t x ) { return (x&tag_QNaN) != tag_QNaN; }
bool nilp( value_t x ) { return x == val_nil; }
bool truep( value_t x ) { return x == val_true; }
bool falsep( value_t x ) { return x == val_false; }

bool fixnump( value_t x ) { return gettag( x ) == tag_fixnum; }
bool symp( value_t x ) { return gettag( x ) == tag_symbol; }
bool consp( value_t x ) { return gettag( x ) == tag_cons; }
bool vecp( value_t x ) { return gettag( x ) == tag_vector; }
bool strp( value_t x ) { return gettag( x ) == tag_string; }
bool funp( value_t x ) { return gettag( x ) == tag_function; }

bool listp( value_t x ) { return consp( x ) || nilp( x ); }
bool builtinp( value_t x ) { return funp( x ) && !managedp( x ); }
bool arrayp( value_t x ) { return vecp( x ) || strp( x ); }
bool closurep( value_t x ) { return funp( x ) && managedp( x ); }
bool gensymp( value_t x ) { return symp( x ) && managedp( x ); }

bool forwardp( value_t x ) { return gettag( x ) == tag_forward; }
bool inlinep( value_t x ) { return arrayp( x ) && fixnump( car( x ) ); }

uint *_alen( value_t x ) {
  if (forwardp(car(x)))
    return (uint*)&cdr( x );

  else
    return (uint*)&car( x );
}

void *_adata( value_t x ) {
  if ( forwardp(car(x)) )
    return ptrval( car(x) );

  return &cdr( x );
}

size_t r_size( value_t x ) {
  ulong t = gettag( x );

  switch ( t ) {
  case tag_symbol:
    return sizeof(symbol_t) + fixval( slen( x ) ) + 1;

  case tag_cons:
    return sizeof(cons_t);

  case tag_vector:
    return 8 + asize( x ) * sizeof(value_t);

  case tag_string:
    return 8 + asize( x ) + 1;

  case tag_function:
    if (managedp( x ))
      return (1 + asize( x )) * sizeof(value_t);

    __attribute__((fallthrough));

  default:
    return 8;
  }
}


value_t intern( symbols_t **table, char *name ) {
  hash_t h = strhash( name );

  while ( *table ) {
    int o = ord_uint( h, (*table)->base.hash ) ? : strcmp( name, (*table)->base.name );
    if ( o < 0 )
      table = &(*table)->left;
    else if ( o > 0 )
      table = &(*table)->right;
    else
      break;
  }

  if ( *table == NULL ) {
    int l  = strlen( name );
    symbol_t *out = mk_symbol( name, l+1, h, true );
    
    *table = sentry( out );
    (*table)->left = (*table)->right = NULL;
  }

  return settag( &((*table)->base), tag_symbol ); 
}

symbol_t *mk_symbol( char *n, int l, hash_t h, bool i ) {
  symbol_t *out = malloc_s( sizeof(cons_t) * i + sizeof(symbol_t) + l );

  out->hash = h;
  out->idno = Symcnt++;
  out->bind = Unbound;
  strcpy( out->name, n );
  out->len = settag( l, tag_fixnum );

  return out;
}

vector_t *mk_vector( int n ) {
  
}

inline bool Cbool( value_t x ) { return !(x == val_nil || x == val_false); }

inline value_t mk_num( double x ) { return ((ieee64_t)x).bin; }
inline value_t mk_fixnum( long x ) { return settag( x, tag_fixnum ); }
inline value_t mk_bool( int x ) { return x ? val_true : val_false; }
