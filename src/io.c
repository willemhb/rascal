#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "io.h"
#include "object.h"
#include "runtime.h"
#include "vm.h"

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
static value_t  take( FILE *ios );
static int      accumc( int ch );
static int      skipc( FILE *ios );

static void clear_reader( void );

static void    getsymtok( FILE *ios, int ch );
static value_t readexpr( FILE *ios );
static value_t read_sexpr( FILE *ios );
static value_t read_vector( FILE *ios );

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

static void clear_reader( void ) {
  memset( token_buffer, 0, bufi );
  bufi        = 0;
  token       = tok_ready;
  token_value = val_nil;
}

static value_t take( FILE *ios ) {
  (void)ios;
  
  value_t out = token_value;
  clear_reader();
  return out;
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
    take( ios );
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
  return (token_value = pop());
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
  return (token_value=pop());
}

static value_t read_binary( FILE *ios ) {
  Ctype_t ctype; size_t n = 0;
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

  take( ios );

  index_t base = Sp;
  
  while ((tok = get_token( ios )) != tok_rbrack ) {
    require( "read",
	     tok != tok_eof,
	     "unexpected EOF reading #" );

    x = readexpr( ios );
    push(x);
    n++;
  }

  take( ios );

  uchar buf[n*elsize];

  index_t b_bufi = 0;

  for (size_t i=0; i<n; i++) {
    value_t x = Stack[base+i];
    fixnum_init( "binary", x, ctype, &buf[b_bufi] );
    b_bufi += elsize;
  }

  popn(n);

  binary_s(n, ctype, buf);

  return (token_value = pop());
}

static value_t read_macro( FILE *ios, token_t tok ) {
  value_t macro_sym = val_nil, out = val_nil;
  
  if (tok == tok_quote) macro_sym = r_quote;
  else
    error( "read",
	   "Unknown macro character '%s'",
	   token_names[tok] );

  assert( macro_sym != val_nil );
  take( ios );

  out = readexpr( ios );
  out = list2( macro_sym, out );
  return (token_value = out);
}

value_t readexpr( FILE *ios ) {
  // assert( scanner.token == tok_ready && "scanner not cleared." );

  token_t tok = get_token( ios );

  assert(tok != tok_ready && "get_token() didn't read anything." );

  value_t out = val_nil;

  switch (tok) {
  case tok_eof: break;
  case tok_lpar:
    take( ios );
    read_sexpr( ios );
    out = take( ios );
    break;

  case tok_lbrack:
    take( ios );
    read_vector( ios );
    out = take( ios );
    break;

  case tok_hash:
    take( ios );
    read_binary( ios );
    out = take( ios );
    break;

  case tok_quote:
    read_macro( ios, tok );
    out = take( ios );
    break;

  case tok_symbol ... tok_nil:
    out = take( ios );
    break;

  case tok_dot: case tok_rpar: // syntax error
    error( "read",
	    "unexpected '%s' token",
	    token_names[tok] );
    break;

  case tok_ready: default:
    __builtin_unreachable( );
  }

  return out;
}

value_t r_read( FILE *ios ) { // API for readexpr
  assert( token == tok_ready );

  clear_reader();

  value_t out = readexpr( ios );

  return out;
}

value_t r_load( char *fname ) {
  assert( token == tok_ready );

  FILE *ios = fopen( fname, "rt" );
  value_t out = val_nil;

  require( "load",
	   ios,
	   strerror( errno ) );

  clear_reader();

  int saveSp = Sp, saveFp = Fp, savePc = Pc, saveBp = Bp;

  if (setjmp(Toplevel)) {
    fprintf( stderr, "aborting to toplevel.\n" );
    out = val_nil;
  }

  else {
    while ((token = get_token(ios)) != tok_eof) {
      value_t xpr  = readexpr( ios );
      value_t code = compile( xpr );
      out = execute( code );
    }
  }

  Sp = saveSp;
  Fp = saveFp;
  Pc = savePc;
  Bp = saveBp;

  clear_reader();
  fclose( ios );

  return out;
}

static size_t prin_cons( FILE *ios, value_t c) {
  size_t out = 2;

  fputc( '(', ios );

  while (is_cons(c)) {
    value_t x = car(c);
    out += r_prin(ios, x);

    if (is_cons(cdr(c))) {
      out++;
      fputc( ' ', ios );
    }

    c = cdr(c);
  }

  if (!is_nil(c)) {
    fputs(" . ", ios );
    out += 3;

    out += r_prin( ios, c );
  }

  fputc( ')', ios );
  return out;
}

static size_t prin_vector( FILE *ios, value_t v) {
  size_t out = fprintf( ios, "[" );

  if (!is_empty(v)) {
    value_t *vals = adata(v);
    size_t cap = alength(v);

    for (size_t i=0; i<cap; i++) {
      out += r_prin( ios, vals[i] );

      if (i+1 < cap)
	out += fprintf( ios, " " );
    }
  }

  return out + fprintf( ios, "]" );
}

static size_t prin_binary( FILE *ios, value_t b ) {
  Ctype_t ctype = get_Ctype(b);
  size_t out = fprintf( ios, "#%s[", Ctype_names[ctype]);

  if (!is_empty(b)) {

  }

  
}

size_t r_prin( FILE *ios, value_t x ) {
  
}



// initialization -------------------------------------------------------------
void init_io( void ) {}
