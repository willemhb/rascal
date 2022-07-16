#include <assert.h>
#include <string.h>

#include "numutils.h"
#include "memutils.h"
#include "ctypes.h"
#include "strutils.h"
#include "hashing.h"

#include "object.h"
#include "memory.h"
#include "error.h"
#include "stack.h"

mk_val_p(true)
mk_val_p(false)
mk_val_p(nil)

mk_tag_p(immediate)
mk_tag_p(object)
mk_tag_p(fixnum)
mk_tag_p(cons)
mk_tag_p(symbol)
mk_tag_p(function)

mk_type_p(character)
mk_type_p(bool)
mk_type_p(builtin)
mk_type_p(closure)
mk_type_p(string)
mk_type_p(bytecode)
mk_type_p(vector)

mk_safe_cast(cons, cons_t*, ascons)
mk_safe_cast(symbol, symbol_t*, assymbol)
mk_safe_cast(vector, vector_t*, asvector)
mk_safe_cast(string, string_t*, asstring)
mk_safe_cast(bytecode, bytecode_t*, asbytecode)
mk_safe_cast(closure, closure_t*, asclosure)
mk_safe_cast(builtin, builtin_t, asbuiltin)
mk_safe_cast(bool, boolean_t, ival)
mk_safe_cast(fixnum, fixnum_t, ival)

bool gensymp( value_t x ) {
  return symbolp( x ) && !managedp( x );
}

bool keywordp( value_t x ) {
  return symbolp( x ) && sname( x )[0] == ':';
}

type_t r_type( value_t x ) {
  int t;
  switch ( (t=gettag( x )) ) {
  case tag_fixnum:
  case tag_cons:
  case tag_symbol:
    return t;

  case tag_immediate:
    return x>>25;

  case tag_function:
    return managedp( x ) ? type_closure : type_builtin;

  case tag_object:
  default:

    return type( x );
  }
}

void *_odata( value_t x ) {
  if ( inlined( x ) )
    return pval( cdr( x ) );

  return &cdr( x );
}

static size_t r_asize( value_t x ) {
  size_t base = sizeof(object_t) + offset( x ) * sizeof(value_t);

  if ( inlined( x ) )
    base += ( length( x ) + !!encoding( x ) ) * Ctype_size( Ctype( x ) );

  return base;
}

size_t r_size( value_t x ) {
  switch ( r_type( x ) ) {
  case type_cons:
    return sizeof(cons_t);

  case type_symbol:
    return sizeof(symbol_t) + strlen( sname( x ) );

  case type_closure:
    return sizeof(closure_t);

  case type_fixnum: case type_nil: case type_builtin:
    return 8;

  case type_bool:
    return 4;
    
  case type_character:
    return 1;

  default:
    return r_asize( x );
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

  return  (value_t)&((*table)->base);
}

static int init_object( object_t *ob, type_t t, bool i ) {
  static const bool boxedp[type_pad] = {
    [type_cons]    = true, [type_symbol]   = false,
    [type_vector]  = true, [type_string]   = false,
    [type_closure] = true, [type_bytecode] = false
  };

    static const  Ctype_t Ctype[type_pad] = {
    [type_cons]    = C_sint64, [type_symbol]   = C_sint8,
    [type_vector]  = C_sint64, [type_string]   = C_sint8,
    [type_closure] = C_sint64, [type_bytecode] = C_sint16
  };


    static const  encoding_t encoding[type_pad] = {
    [type_symbol]   = enc_ascii, [type_string]  = enc_ascii,
  };

  
  ob->type = t;
  ob->Ctype = Ctype[t];
  ob->encoding = encoding[t];
  ob->inlined = i;
  ob->boxed = boxedp[t];

  return 0;
}

static int init_big_object( big_object_t *ob, type_t t, bool i, size_t l, size_t c ) {
  init_object( &ob->base, t, i );

  ob->length = l;
  ob->size = c;

  return 0;
}

static int init_string( string_t *ob, char *data, bool i, size_t l ) {
  char * dst = &ob->data[0];

  if ( !i )
    dst = *(char**)dst;

  if ( data )
    strncpy( dst, data, l );
  
  init_big_object( (big_object_t*)ob, type_string, i, l, arr_resize( l ) + 1 );

  return 0;
}


static int init_vector( vector_t *ob, value_t *data, bool i, size_t l ) {
  value_t * dst = &ob->data[0];

  if ( !i )
    dst = *(value_t**)dst;

  if ( data )
    memcpy( dst, data, l * sizeof( value_t ) );
  
  init_big_object( (big_object_t*)ob, type_string, i, l, arr_resize( l ) );

  return 0;
}


symbol_t *mk_symbol( char *n, size_t l, hash_t h, bool i ) {
  static const char* gensym_format = "%s#%lu";
  symbol_t *out;
  char buf[l+1];
  strcpy( buf, n );

  if ( i ) {
    symbols_t *spc = malloc_s( sizeof(symbols_t) + l + 1 );
    out = &spc->base;

  } else {
    size_t bufsize = l + 2 + SAFE_NUMBER_BUFFER_SIZE;
    char hbuf[bufsize];
    snprintf( hbuf, bufsize, gensym_format, n, Symcnt+1 ); // ensure reasonable hash
    h = strhash( hbuf );
    out = allocate( sizeof(symbol_t) + l + 1 );
  }

  strcpy( out->name, buf );

  out->hash  = h;
  out->idno  = Symcnt++;
  out->bind  = buf[0] == ':' ? settag( out, tag_symbol ) : Unbound;
  out->flags = tag_fixnum | symfl_const * ( buf[0] == ':');

  return out;
}

value_t symbol( char *name ) {
  if ( name == NULL )
    return gensym( "symbol" );

  assert( *name != '\0' );
  
  return intern( &Symbols, name );
}

value_t gensym( char *name ) {
  if ( name == NULL )
    name = "symbol";

  assert( *name != '\0' );

  size_t l = strlen( name );
  symbol_t *new = mk_symbol( name, l, 0, false );
  return settag( new, tag_symbol );
}

cons_t *mk_cons( void ) {
  return allocate( sizeof(cons_t) );
}

vector_t *mk_vector( size_t n ) {
  size_t total = sizeof( vector_t ) + arr_resize( n ) * sizeof(value_t);

  vector_t *out = allocate( total );
  init_vector( out, NULL, true, n );
  return out;
}

string_t *mk_string( size_t n ) {
  size_t total = sizeof( string_t ) + arr_resize( n ) + 1;

  string_t *out = allocate( total );
  init_string( out, NULL, true, n );
  return out;
}

closure_t *mk_closure( void ) {
  return allocate( sizeof( closure_t ) );
}

inline bool Cbool( value_t x ) { return !(x == val_nil || x == val_false); }

value_t mk_fixnum( long x ) { return ((value_t)x) << 2; }
value_t mk_bool( int x ) { return x ? val_true : val_false; }
value_t mk_character( int c ) { return (((value_t)c) << 2) | tag_character; }

value_t *cons_s( value_t *ca, value_t *cd ) {

  cons_t *ob = mk_cons();
  ob->car = *ca;
  ob->cdr = *cd;
  value_t tagged = settag( ob, tag_cons );

  if ( ca == &Peek( 1 ) && cd == &Peek( 2 ) ) // typical use case but others conceivable
    Stack[--Sp] = tagged;

  else
    push( tagged );

  return &Tos;
}

value_t *vector_s( size_t n, value_t *a ) {
  vector_t *ob = mk_vector( n );

  memcpy( &ob->data[0], a, n * sizeof(value_t) );
  value_t tagged = settag( ob, tag_object );

  if ( a + n == &Tos )
    Stack[(Sp = Sp - n + 1)] = tagged;

  else
    push( tagged );

  return &Tos;
}

value_t ncat( value_t x, value_t y ) {
  if ( nilp( x ) )
    return y;

  if ( nilp( y ) )
    return x;

  value_t tmp = x;
  
  while ( consp(cdr( tmp )) ) tmp = cdr( tmp );

  cdr( tmp ) = y;

  return x;
}

value_t nrev( value_t x ) {
  if ( nilp( x ) )
    return x;

  value_t tmp = x, prev = val_nil;

  while ( consp( tmp ) ) {
    value_t swap = cdr( tmp );
    cdr( tmp )   = prev;
    prev         = tmp;
    tmp          = swap;
  }

  return prev;
}

value_t assoc( value_t k, value_t xs ) {
  while ( consp( xs ) ) {
    if ( caar( xs ) == k )
      return car( xs );

    xs = cdr( xs );
  }

  return xs;
}
