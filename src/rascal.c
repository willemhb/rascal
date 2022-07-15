// standard headers
#include <string.h>
#include <assert.h>

// utilties
#include "memutils.h"
#include "numutils.h"
#include "hashing.h"

// core
#include "rascal.h"

// runtime system
#include "stack.h"
#include "error.h"

// declare globals
ulong Symcnt = 0;
symbols_t *Symbols = NULL;

// stack & registers
value_t Stack[N_STACK];
int Sp = 0, Fp = 0, Bp = 0, pc = 0;

uchar *Heap, *Reserve, *Free;

size_t  HSize = N_STACK*sizeof(cons_t),
        HUsed = 0,
        RSize = N_STACK*sizeof(cons_t),
        RUsed = 0;

bool  Collecting = false,
      Grow       = false,
      Grew       = false;

float Collectf   = 1.0,
      Resizef    = 0.685,
      Growf      = 2.0;

// utilities ------------------------------------------------------------------
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

bool movedp( value_t x ) {
  if (forwardp(car( x )))
    return withinp( ptrval( car(x) ), Heap, HUsed );

  return false;
}

// memory management ----------------------------------------------------------

symbol_t *mk_symbol( char *n, int l, hash_t h, bool i );

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

value_t symbol( char *name );


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

// virtual machine ------------------------------------------------------------
inline bool Cbool( value_t x ) { return !(x == val_nil || x == val_false); }

inline value_t mk_num( double x ) { return ((ieee64_t)x).bin; }
inline value_t mk_fixnum( long x ) { return settag( x, tag_fixnum ); }
inline value_t mk_bool( int x ) { return x ? val_true : val_false; }
// inline value_t mk_char( int c ) { return tagi( c, type_character ); }

int r_opargc( int op ) {
  if ( op < op_loadval )
    return 0;

  else if ( op < op_done )
    return 1;

  else
    return 0;
}

value_t r_exec( int n ) {
  static void *labels[num_instructions] = {
    // misc
    [op_noop]=&&do_noop,
    // stack manipulation
    [op_push]=&&do_push, [op_pop]=&&do_pop,
    // loads
    [op_loadval]  = &&do_loadval,   [op_loadgl]   = &&do_loadgl,
    [op_loadloc]  = &&do_loadloc,   [op_loadupv]  = &&do_loadupv,
    // stores
    [op_storegl]  = &&do_storegl,   [op_storeloc] = &&do_storeloc,
    [op_storeupv] = &&do_storeupv,
    // control flow
    [op_jump]     = &&do_jump,      [op_jumpt]    = &&do_jumpt,
    [op_jumpf]    = &&do_jumpf,
    // guards
    [op_argc]     = &&do_argc,
    [op_vargc]    = &&do_vargc,
    // function calls & envt
    [op_call]     = &&do_call,      [op_return] = &&do_return,
    [op_closure]  = &&do_closure,   [op_capture]= &&do_capture,
    
    // misc
    [op_done]     = &&do_done
  };
  
  int op, argx, argy;
  value_t valx, valy;

 do_fetch:
  op = Instr[Pc++];

  if ( r_opargc( op ) )
    argx = Instr[Pc++];
  
  goto *labels[op];
  
 do_noop:
  goto do_fetch;

 do_push:


  push( valx );
  
  goto do_fetch;

 do_pop:
  valx = pop();

  goto do_fetch;

 do_loadval:
  valx = Value[argx];
  
  goto do_push;

 do_loadgl:
  valy = Value[argx];
  valx = sbind( valy );

  goto do_push;

 do_loadloc:
  valx = Stack[Bp+argx];

  goto do_push;

 do_loadupv:
  valx = Upval[argx];
  
  if ( forwardp( valx ) )
    valx = Stack[idxval(valx)];

  goto do_push;

 do_storegl:
  valx = Value[argx];
  sbind( valx ) = Tos;

  goto do_fetch;

 do_storeloc:
  Stack[Bp+argx] = Tos;

  goto do_fetch;

 do_storeupv:
  valx = Upval[argx];

  if ( forwardp( valx ) )
    Stack[idxval(valx)] = Tos;

  else
    Upval[argx] = Tos;

  goto do_fetch;

 do_jump:
  Pc += argx;

  goto do_fetch;

 do_jumpt:
  valx = pop();

  if ( Cbool( valx ) )
    Pc += argx;

  goto do_fetch;

 do_jumpf:
  valx = pop();

  if ( !Cbool( valx ) )
    Pc += argx;

  goto do_fetch;

  /* guards */

 do_argc:
  require( r_fname(),
	   longval( Argc ) == argx,
	   "# expected %d arguments, got %dl",
	   argx,
	   longval( Argc ) );

  goto do_fetch;

 do_vargc:
  require( r_fname(),
	   longval( Argc ) >= argx,
	   "# expected at least %d arguments, got %dl",
	   argx,
	   longval( Argc ) );

 do_capture:
  valx = Upval[argx];

  if ( forwardp( valx ) ) { // make idempotent I guess
    argy = idxval( valx );
    Upval[argx] = Stack[argy];
  }

  goto do_fetch;

 do_closure: // create a new closure
  if (nilp( Env )) {
    
  }
  
 do_return:
  
 do_call: // call the procedure at Stack[Sp-argx-1]
  valx = Peek(argx-1);

  require( r_fname(),
	   funp( valx ),
	   "apply expected a function(), got something else" );

  if (builtinp(valx)) { // callback manages stack itself
    ((builtin_t)ptrval( valx ))( argx );

    goto do_fetch;
  }

  argy = Sp; // needed in a minute to set frame pointer

  /* save calling frame */
  push( clenvt( valx ) );
  push( mk_fixnum( Fp ) );
  push( mk_fixnum( argx ) );
  push( mk_fixnum( Pc ) );

  /* set up local frame */
  Fp    = argy;
  Pc    = 0;
  Bp    = Fp - argx;
  Instr = adata( clcode( Func ) );
  Value = adata( clvals( Func ) );
  Upval = envupv( Env );

  /* begin execution */
  goto do_fetch;

 do_done:
  valx = pop();

  /* TODO: cleanup */
  return valx;
}
 
static void rascal_init( void ) {
}

int main( int argc, const char **argv ) {
  (void)argc;
  (void)argv;

  rascal_init( );

  return 0;
}
