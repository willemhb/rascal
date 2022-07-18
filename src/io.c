#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "io.h"
#include "object.h"
#include "runtime.h"

#define atm_maxlen 8192
#define buf_inisz  512
#define buf_grow_factor 2
#define prompt_in  "<< "
#define prompt_out ">> "

static char *Ctype_names[] = {
  [C_sint8]   = "s8",  [C_uint8]   = "u8",
  [C_sint16]  = "s16", [C_uint16]  = "u16",
  [C_sint32]  = "s32", [C_uint32]  = "u32",
  [C_sint64]  = "s64", [C_float64] = "f64"
};


static char *token_names[] = {
  [tok_lpar] = "(",     [tok_rpar] = ")",
  
  [tok_lbrack] = "[",     [tok_rbrack] = "]",
  
  [tok_hash] = "#", [tok_quote] = "'", [tok_dot] = ".",
  
  [tok_symbol] = "symbol", [tok_integer] = "integer",
  [tok_true]   = "true",   [tok_false] = "false",
  [tok_nil]    = "nil",

  [tok_eof] = "EOF", [tok_ready] = "ready"
};

// static helpers -------------------------------------------------------------
#define bufmax 2048
#define streql(x,y) (strcmp(x,y)==0)

static char token_buffer[bufmax+1];
static int bufi = 0;
static token_t token;
static value_t token_value;

static inline bool dlmchr(int c);
static inline bool symchr(int c);

static int      peekc( FILE *ios );
static int      takec( FILE *ios );
static value_t  take( void );
static int      accumc( int ch );
static int      skipc( FILE *ios );

static void clear_reader( void );

static void    getsymtok( FILE *ios, int ch );
static value_t readexpr( FILE *ios );
static value_t read_sexpr( FILE *ios );
static value_t read_vector( FILE *ios );

static size_t fixnum_fits( value_t f, Ctype_t c );



static void getsymtok( FILE *ios, int ch ) {
  int c = accumc( ch );
  bool nump = c != EOF && strchr("+-0123456789", c) != NULL;

  while ((c = peekc( ios )) != EOF && !isspace(c) && !dlmchr(c)) {
    accumc( c );
    nump = nump && isdigit(c);
  }
  
  nump = nump && ( bufi > 1 || isdigit( token_buffer[0] ) );

  if (nump)
    token = tok_integer;

  else if (streql(token_buffer, "true"))
    token = tok_true;

  else if (streql(token_buffer, "false"))
    token = tok_false;

  else if (streql(token_buffer, "nil"))
    token = tok_nil;
  
  else
    token = tok_symbol;

  if ( token == tok_integer ) {
    char *sbuf;
    long l = strtol( token_buffer, &sbuf, 0 );

    assert( *sbuf == '\0' && "integer token failed to read integer" );

    token_value = fixnum( l );

  } else if (token == tok_true) {
    token_value = val_true;
    
  } else if (token == tok_false) {
    token_value = val_false;

  } else if (token == tok_nil) {
    token_value = val_nil;
    
  } else {
    assert( bufi > 0 ); // at least one character should have been read

    require( "read",
	     bufi > 1 || token_buffer[0] != ':',
	     "invalid keyword ':'" );
    
    token_value = symbol( token_buffer );
  }
}

void clear_reader( void ) {
  memset( token_buffer, 0, bufi );
  bufi        = 0;
  token       = tok_ready;
  token_value = val_nil;
}

static token_t get_token( FILE *ios ) {
  if ( token == tok_ready ) {
    int c = skipc( ios );
    
    switch (c) {
    case EOF: token  = tok_eof; break;
    case '(': token  = tok_lpar; takec( ios ); break;
    case ')': token  = tok_rpar; takec( ios ); break;
    case '[': token  = tok_lbrack; takec( ios ); break;
    case ']': token  = tok_rbrack; takec( ios ); break;
    case '\'': token = tok_quote; takec( ios ); break;
    case '.': token = tok_dot; takec( ios ); break;
    case '#': token = tok_hash; takec( ios ); break;
    default: getsymtok( ios, c ); break;
    }
  }

  return token;
}

static value_t read_sexpr( FILE *ios ) {
  bool terminalp( token_t tok ) {
    return tok != tok_rpar && tok != tok_dot;
  }
  
  int n = 0, base;
  base = Sp;

  token_t tok;

  value_t (*constructor)(size_t n, value_t *args ) = list_s;
  
  while ( !terminalp((tok = get_token( ios )))) {
    require( "read",
	     tok != tok_eof,
	     "unexpected EOF reading #" );

    value_t head = readexpr( ios );
    n++;

    push( head );
  }

  if ( tok == tok_dot ) {
    take( );
    tok = get_token( ios );

    switch ( tok ) {
    case tok_dot: case tok_lpar: case tok_eof:
      error( "read",
	     "unexpected '%s' reading s-expression",
	     token_names[token] );
      break;
    default: break;
    }

    // TODO: more validation
    value_t tail = readexpr( ios );
    push( tail );
    n++;
    constructor = consn_s;
  }

  require( "read",
	   token == tok_rpar,
	   "unexpected token doesn't match '(': %s",
	   token_names[token] );

  constructor( n, &Stack[base]);
  token_value = pop();

  return token_value;
}

static value_t read_vector( FILE *ios ) {
  index_t base = Sp;
  size_t n = 0;
  token_t tok;

  while ((tok = get_token(ios)) != tok_rbrack) {
    require( "read",
	     tok != tok_eof,
	     "unexpected EOF reading #" );

    value_t element = readexpr(ios);
    push( element );
    n++;
  }

  vector_s( n, &Stack[base]);
  return pop();
}

static value_t read_binary( FILE *ios ) {
  int ch; Ctype_t ctype; size_t n = 0;
  size_t elsize;
  token_t tok;

  tok = get_token( ios );

  require("read",
	  tok == tok_symbol,
	  "invalid C type token '%s':'%s'",
	  token_names[tok],
	  token_buffer );

  value_t x = readexpr( ios );

  /* TODO: validate the symbol name further */
  ctype = assymbol(x)->bind;
  elsize = Ctype_size( ctype );

  tok = get_token( ios );

  require("read",
	  tok == tok_lbrack,
	  "invalid token reading #: '%s'",
	  token_buffer );

  take();

  index_t base = Sp;
  
  while ((tok = get_token( ios )) != tok_rbrack ) {
    require( "read",
	     tok != tok_eof,
	     "unexpected EOF reading #" );

    x = readexpr( ios );

    require( "read",
	     fixnum_fits( x, ctype ),
	     "binary literal doesn't match declared type" );

    push(x);
    n++;
  }

  take();

  uchar buf[n*elsize];

  index_t b_bufi = 0;

  for (size_t i=0; i<n; i++) {
    fixnum_t fx = ival( Stack[base+i] );
    memcpy( buf+b_bufi, &fx, elsize );
    b_bufi += elsize;
  }

  popn(n);

  binary_s(n, ctype, buf);

  return pop();
}

static value_t read_macro( scanner_t *scn, token_t tok ) {
  value_t macro_sym = rs_none, out = rs_none;
  
  if (tok == tok_quote) macro_sym = rs_quote;
  else error( "read",
	      err_value,
	      "Unknown macro character '%s'",
	      rs_none,
	      toknames[tok] );

  assert( macro_sym != rs_none );
  take( scn );
  
  out = readexpr( scn );
  out = list2( macro_sym, out );
  return out;
}

value_t readexpr( scanner_t *scn ) {
  // assert( scanner.token == tok_ready && "scanner not cleared." );
  
  token_t tok = get_token( scn );

  assert(tok != tok_ready && "get_token() didn't read anything." );

#ifdef rsc_debug
  // debug( "token: %s", toknames[tok] );
  // show_val( scn->value );
#endif

  value_t out = rs_nil;

  switch (tok) {
  case tok_eof: break;
  case tok_open:
    take( scn );
    read_sexpr( scn );
    out = take( scn );
    break;

  case tok_quote:
    out = read_macro( scn, tok );
    break;

  case tok_integer ... tok_string:
    out = take( scn );
    break;

  case tok_dot: case tok_close: // syntax error
    error( "read",
	    err_syntax,
	    "unexpected '%s' token",
	    rs_none,
	    toknames[tok] );
    break;

  case tok_ready: default:
    unreachable( );
    break;
  }

#ifdef rsc_debug
  //  printf( "%s: %d: %s: output: ",
  //	  __FILE__,
  //	  __LINE__,
  //	  __func__ );
  // print( stdout, out );
  // printf( ".\n" );
#endif

  return out;
}

value_t read( FILE *fl ) { // API for readexpr
  assert( Scanner.token == tok_ready && "rcread() called while other stream being read." );

  reset_scanner( &Scanner, fl, "<file>" );

  value_t out = readexpr( &Scanner );

  return out;
}

value_t load( char *fname ) {
  assert( Scanner.token == tok_ready && "rcload() called while other stream being read." );

  FILE *fl = fopen( fname, "rt" );
  value_t out = rs_nil;

  require( "load",
	   fl != NULL,
	   strerror( errno ),
	   rs_none );

  reset_scanner( &Scanner, fl, fname );

  int  saveSp;
  savesp( &saveSp );

  switch( setjmp( toplevel ) ) {
  case err_okay:
    while ( get_token( &Scanner ) != tok_eof ) {
      value_t x  = readexpr( &Scanner );

      out        = eval( x, 0 ); // top level context
    }

    break;

  case err_exit:
    break;

  default:
    fprintf( stderr,
	     "load() encountered an error near line %d, in %s, exiting.\n",
	     Scanner.line,
	     Scanner.fname );
    out = rs_nil;
    break;
  }
  
  restoresp( &saveSp );

  reset_scanner( &Scanner, NULL, NULL );
  fclose( fl );

  return out;
}

void repl(void) {
  reset_scanner( &Scanner, NULL, NULL );
  int saveSp;
  value_t x, v;
  
  while (true) {
    fprintf(stdout, "\n%s", prompt_in);

    savesp( &saveSp );

    switch ( setjmp( toplevel ) ) {
    case err_okay: {
      x = readexpr( &Scanner );
      
      if ( x == rs_nil && Scanner.token == tok_eof ) {
	take( &Scanner );
	break;
      }
      
      v = eval( x, 0 );
      
      fprintf( stdout, "\n%s", prompt_out );
      print( stdout, v );
      break;
    }
      
    case err_exit: {
      return;
    }

    default:    
      fprintf( stdout, "recovering.\n" );
      restoresp( &saveSp );
      clear_scanner( &Scanner );
      break;
    }
  }
}

// initialization -------------------------------------------------------------
void init_scanner( void ) {
#ifdef rsc_debug
  fprintf( stdout, "Initializing scanner.\n" );
#endif

  scanner_t *scn = &Scanner;

  scn->buf     = malloc_s( buf_inisz );
  scn->bufsz   = buf_inisz;
  scn->bufi    = 0;

  reset_scanner( scn, NULL, NULL );

#ifdef rsc_debug
  fprintf( stdout, "Scanner initialized.\n" );
#endif
}

static void init_port_type( void ) {
  PortType = newtype( "port" );

  init_type_required( PortType, AnyType, type_port, tag_object, sizeof( port_t ) );
  init_type_valmethods( PortType, print_port, order_port );
}

static void init_streams( void ) {
  rs_ins  = port( stdin );
  rs_outs = port( stdout );
  rs_errs = port( stderr );

  // bind to rascal names
  constant( "*ins*", rs_ins );
  constant( "*outs*", rs_outs );
  constant( "*errs*", rs_errs );
}

static void init_reader_macros( void ) {
  rs_quasiquote = symbol( "quasiquote" );
  rs_unquote    = symbol( "unquote" );
  rs_splice     = symbol( "unquote-splice" );
}

// initialization -------------------------------------------------------------
void reader_init( void ) {
#ifdef rsc_debug
  fprintf( stdout, "Initializing reader.\n" );
#endif

  init_scanner();
  init_port_type();
  init_streams();
  init_reader_macros();

#ifdef rsc_debug
  fprintf( stdout, "Reader initialized.\n" );
#endif
}
