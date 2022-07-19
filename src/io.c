#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include "io.h"
#include "object.h"
#include "runtime.h"
#include "symbol.h"
#include "list.h"
#include "function.h"
#include "array.h"
#include "number.h"
#include "vm.h"

#define atm_maxlen 8192
#define buf_inisz  512
#define buf_grow_factor 2
#define prompt_in  "<< "
#define prompt_out ">> "

static char *token_names[] = {
  [tok_lpar] = "(",     [tok_rpar] = ")",
  
  [tok_lbrack] = "[",     [tok_rbrack] = "]",
  
  [tok_hash] = "#", [tok_quote] = "'", [tok_dot] = ".",
  
  [tok_symbol] = "symbol", [tok_integer] = "integer",
  [tok_true]   = "true",   [tok_false] = "false",
  [tok_nil]    = "nil",    [tok_character] = "character",
  [tok_string] = "string",

  [tok_eof] = "EOF", [tok_ready] = "ready"
};

static char* chrnames[CHAR_MAX+1] = {
    ['\0']   = "nul",       ['\x01'] = "soh",
    ['\x02'] = "stx",       ['\x03'] = "etx",
    ['\x04'] = "eot",       ['\x05'] = "enq",
    ['\x06'] = "ack",       ['\a']   = "bel",
    ['\b']   = "backspace", ['\t']   = "tab",
    ['\n']   = "newline",   ['\v']   = "vtab",
    ['\f']   = "formfeed",  ['\r']   = "return",
    ['\x0e'] = "ss",        ['\x0f'] = "si",
    ['\x10'] = "dle",       ['\x11'] = "dc1",
    ['\x12'] = "dc2",       ['\x13'] = "dc3",
    ['\x14'] = "dc4",       ['\x15'] = "nak",
    ['\x16'] = "syn",       ['\x17'] = "etb",
    ['\x18'] = "can",       ['\x19'] = "em",
    ['\x1a'] = "sub",       ['\e']   = "escape",
    ['\x1c'] = "fs",        ['\x1d'] = "gs",
    ['\x1e'] = "rs",        ['\x1f'] = "us",
    [' ']    = "space",     ['!']    = "!",
    ['\"']   = "\"",        ['#']    = "#",
    ['$']    = "$",         ['%']    = "%",
    ['&']    = "&",         ['\'']   = "'",
    ['(']    = "(",         [')']    = ")",
    ['*']    = "*",         ['+']    = "+",
    [',']    = ",",         ['-']    = "-",
    ['.']    = ".",         ['/']    = "/",
    ['0']    = "0",         ['1']    = "1",
    ['2']    = "2",         ['3']    = "3",
    ['4']    = "4",         ['5']    = "5",
    ['6']    = "6",         ['7']    = "7",
    ['8']    = "8",         ['9']    = "9",
    [':']    = ":",         [';']    = ";",
    ['<']    = "<",         ['?']    = "?",
    ['@']    = "@",         ['A']    = "A",
    ['B']    = "B",         ['C']    = "C",
    ['D']    = "D",         ['E']    = "E",
    ['F']    = "F",         ['G']    = "G",
    ['H']    = "H",         ['I']    = "I",
    ['J']    = "J",         ['K']    = "K",
    ['L']    = "L",         ['M']    = "M",
    ['N']    = "N",         ['O']    = "O",
    ['P']    = "P",         ['Q']    = "Q",
    ['R']    = "R",         ['S']    = "S",
    ['T']    = "T",         ['U']    = "U",
    ['V']    = "V",         ['W']    = "W",
    ['X']    = "X",         ['Y']    = "Y",
    ['Z']    = "Z",         ['[']    = "[",
    ['\\']   = "\\",        [']']    = "]",
    ['^']    = "^",         ['_']    = "_",
    ['`']    = "`",         ['a']    = "a",
    ['b']    = "b",         ['c']    = "c",
    ['d']    = "d",         ['e']    = "e",
    ['f']    = "f",         ['g']    = "g",
    ['h']    = "h",         ['i']    = "i",
    ['j']    = "j",         ['k']    = "k",
    ['l']    = "l",         ['m']    = "m",
    ['n']    = "m",         ['o']    = "o",
    ['p']    = "p",         ['q']    = "q",
    ['r']    = "r",         ['s']    = "s",
    ['t']    = "t",         ['u']    = "u",
    ['v']    = "v",         ['w']    = "w",
    ['x']    = "x",         ['y']    = "y",
    ['z']    = "z",         ['{']    = "{",
    ['}']    = "}",         ['|']    = "|",
    ['~']    = "~",         ['\x7f'] = "del",
    ['=']    = "=",         ['>']    = ">",
};

static struct { char code; char* name; } char_trie['v'+1][7] = {
  ['a'] = { { '\x06', "ack" } },
  
  ['b'] = { { '\b', "backspace" },
	    { '\a', "bel" } },
  
  ['c'] = { { '\x18', "can" } },

  ['d'] = { { '\x10' , "dle" },
	    { '\x11' , "dc1" },
	    { '\x12' , "dc2" },
	    { '\x13' , "dc3" },
	    { '\x14' , "dc4" },
	    { '\x7f', "del" } },
  
  ['e'] = { { '\x05', "enq" },
	    { '\x04', "eot" },
	    { '\x03', "etx" },
	    { '\x17', "etb" },
	    { '\x19', "em" },
	    { '\e',   "escape" } },

  ['f'] = { { '\f', "formfeed" },
	    { '\x1c', "fs" } },

  ['g'] = { {  '\x1d', "gs" } },
  
  ['n'] = { { '\n', "newline" },
	    { '\0', "nul" },
	    { '\x15', "nak" } },
  
  ['s'] = { { '\x01', "soh" },
	    { '\x03', "stx" },
            { '\x0f', "si"  },
	    { '\x0e', "ss"  },
	    { '\x16', "syn" },
	    { '\x1a', "sub" },
	    { ' ',    "space"  } },
   
  ['r'] = { { '\r', "return" },
	    { '\x1e', "rs" } },
  
  ['t'] = { { '\t', "tab" } },

  ['u'] = { { '\x1f', "us" } },
  
  ['v'] = { { '\v', "vtab" } },
};

// static helpers -------------------------------------------------------------
#define bufmax 2048
#define streql(x,y) (strcmp(x,y)==0)

static char token_buffer[bufmax+1];
static int bufi = 0;
static token_t token = tok_ready;
static value_t token_value = val_nil;

static inline bool dlmchr(int c);
static inline bool symchr(int c);

static int      peekc( FILE *ios );
static int      takec( FILE *ios );
static value_t  take( FILE *ios );
static int      accumc( int ch );
static int      skipc( FILE *ios );

static void clear_reader( void );

static void    getsymtok( FILE *ios, int ch );
static void    getchrtok( FILE *ios );
static void    getstrtok( FILE *ios );
static value_t readexpr( FILE *ios );
static value_t read_sexpr( FILE *ios );
static value_t read_vector( FILE *ios );

static int peekc( FILE *ios ) {
  int ch = fgetc( ios );

  if (ch != EOF)
    ungetc( ch, ios );

  return ch;
}

static int takec( FILE *ios ) {
  return fgetc( ios );
}

static int accumc( int ch ) {
  require( "read",
	   bufi < bufmax+1,
	   "maximum token length exceeded" );

  token_buffer[bufi++] = ch;

  return ch;
}

static int skipc( FILE *ios ) {
    char c;
    int ch;

    do {
        ch = takec(ios);
        if (ch == EOF)
            return 0;
        c = (char)ch;
        if (c == ';') {
            // single-line comment
            do {
                ch = takec(ios);
                if (ch == EOF)
                    return 0;
            } while ((char)ch != '\n');
            c = (char)ch;
        }
    } while (isspace(c));
    return c;
}

static void getchrtok( FILE *ios ) {
  int ch;

  while ((ch=peekc(ios)) != EOF && !(isspace(ch) || dlmchr(ch))) {
    accumc( ch );
    takec( ios );
  }

  if (bufi == 1) {
    token_value = character( token_buffer[0] );
    return;
  }

  int key = token_buffer[0];

  if (key <= 'v')
    for (size_t i=0; i<7 && char_trie[key][i].name; i++) {
      if (streql(token_buffer, char_trie[key][i].name)) {
	token_value = character( char_trie[key][i].code );
	return;
      }
    }

  error( "read",
	 "unrecognized character '\\%s'",
	 token_buffer );
}

static void getstrtok( FILE *ios) {
  int c;
  bool escape = false;
  static char escape_map[CHAR_MAX+1] = {
    ['b'] = '\b', ['a'] = '\a', ['t'] = '\t',
    ['n'] = '\n', ['v'] = '\v', ['f'] = '\f',
    ['0'] = '\0', ['r'] = '\r'
  };

  while ((c = peekc(ios)) != '"' || escape) {
    require( "read",
	     c != EOF,
	     "unexpected EOF reading #" );

    if (escape) {
      if (c == '0' || strchr( "batnvfr", c ))
	accumc( escape_map[c]);

      else
	accumc(c);

      escape = false;
      takec( ios );
    }

    else if (c == '\\')
      escape = true;

    else
      accumc( takec(ios) );
  }

  takec( ios ); // clear the terminating "
  string_s( bufi, token_buffer );
  token_value = pop();
}

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

    assert( *sbuf == '\0' );

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
    case '(': token  = tok_lpar; break;
    case ')': token  = tok_rpar; break;
    case '[': token  = tok_lbrack; break;
    case ']': token  = tok_rbrack; break;
    case '\'': token = tok_quote; break;
    case '.': token = tok_dot; break;
    case '#': token = tok_hash; break;
    case '"': token = tok_string; getstrtok( ios ); break;
    case '\\': token = tok_character; getchrtok( ios ); break;
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

  case tok_symbol ... tok_string:
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

size_t r_prin( FILE *ios, value_t x ) {
  type_t xt = r_type(x);
  
  switch (xt) {
  case type_fixnum:
    return fprintf( ios, "%ld", ival(x) );
    
  case type_character:
    return fprintf( ios, "\\%s", chrnames[cval(x)]);
    
  case type_type:
    return fprintf( ios, "%s()", Typenames[cval(x)]);
    
  case type_boolean:
    return fprintf( ios, (x == val_true ? "true" : "false" ));

  default:
    if (prin_dispatch[xt])
      return prin_dispatch[xt](ios,x);

    return fprintf( ios, "<%s>", Typenames[xt] );
  }
}

value_t r_comp_file( char *fname ) {
  
}

// builtins -------------------------------------------------------------------
void r_builtin(read) {
  argc( "read", n, 0 );
  value_t out = r_read( stdin );
  push_s( "read", out );
}

void r_builtin(prin) {
  argc( "prin", n, 1 );
  r_prin( stdout, Stack[Sp-1] );
}

void r_builtin(load) {
  
}

// initialization -------------------------------------------------------------
void init_io( void ) {

}
