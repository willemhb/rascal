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
  [tok_lpar]      = "(",           [tok_rpar]      = ")",

  [tok_lbrack]    = "[",           [tok_rbrack]    = "]",
  
  [tok_quote]     = "'",           [tok_dot]       = ".",

  [tok_binary]    = "binary",
  [tok_symbol]    = "symbol",      [tok_function]  = "function",
  [tok_integer]   = "integer",     [tok_true]      = "true",
  [tok_false]     = "false",       [tok_nil]       = "nil",
  [tok_character] = "character",   [tok_string]    = "string",
  
  [tok_eof]       = "EOF",         [tok_ready]     = "ready"
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

// reader state ---------------------------------------------------------------
#define bufmax 2048

static char token_buffer[bufmax+1];
static int bufi = 0;
static token_t token = tok_ready;
static value_t token_value = val_nil;
static Ctype_t token_Ctype = C_sint64; // used by read_binary

// static helpers -------------------------------------------------------------
#define streql(x,y) (strcmp(x,y)==0)

static inline bool dlmchr(int c);
static inline bool symchr(int c);

static int      peekc( FILE *ios );
static int      takec( FILE *ios );
static value_t  take( FILE *ios );
static int      accumc( int ch );
static int      skipc( FILE *ios );

static void clear_reader( void );

static void    get_symtok( FILE *ios, int ch );
static void    get_chrtok( FILE *ios );
static void    get_strtok( FILE *ios );
static value_t read_expr( FILE *ios );

static value_t read_function( FILE *ios );
static value_t read_binary( FILE *ios );
static value_t read_sexpr( FILE *ios );
static value_t read_vector( FILE *ios );

bool is_rascal_source_file( const char *fname ) {
  char *buf = strstr(fname, ".rsp" );

  return buf && buf[4] == '\0';
}

bool is_rascal_data_file( const char *fname ) {
  char *buf = strstr(fname, ".rdn" );

  return buf && buf[4] == '\0';
}

bool is_rascal_object_file( const char *fname ) {
  char *buf = strstr( fname, ".rdn.o" );

  return buf && buf[6] == '\0';
}

bool is_rascal_file( const char *fname ) {
  return is_rascal_source_file( fname ) ||
    is_rascal_data_file( fname ) ||
    is_rascal_object_file( fname );
}

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

static inline bool dlmchr( int c ) {
  return strchr( "[](){}\"", c );
}

static inline bool symchr( int c ) {
  return isalnum(c) ||
    (isprint(c) && !strchr("[](){}\"\\`~@#;", c));
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

static void get_chrtok( FILE *ios ) {
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

static void get_strtok( FILE *ios) {
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

static void get_symtok( FILE *ios, int ch ) {
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
    case '"': token = tok_string; get_strtok( ios ); break;
    case '\\': token = tok_character; get_chrtok( ios ); break;

    case '#':
      while (!dlmchr(c)) {
	c = takec(ios);

	require( "read",
		 c != EOF,
		 "unexpected EOF reading #" );

	accumc(c);
      }

      if ( streql("fun", token_buffer) )
	token = tok_function;

      else
	for (size_t i=0; i<C_float64+1; i++)
	  if ( streql(Ctype_names[i], token_buffer) ) {
	    token = tok_binary;
	    token_Ctype = i;
	    break;
	  }

      require( "read",
	       token != tok_ready,
	       "unrecognized dispatch code '#%s'",
	       token_buffer );

      break;
      
    default: get_symtok( ios, c ); break;
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

    value_t head = read_expr( ios );
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
    value_t tail = read_expr( ios );
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

    value_t element = read_expr(ios);
    push( element );
    n++;
  }

  vector_s( n, &Stack[base]);
  return (token_value=pop());
}

static value_t read_function( FILE *ios ) {
  value_t x = read_expr(ios);
  require( "read",
	   is_list(x),
	   "invalid closure environment reading #" );
  push(x);
  x = read_expr(ios);
  require("read",
	  is_vector(x),
	  "invalid stored values reading #" );
  push(x);
  x = read_expr(ios);
  require("read",
	  is_binary(x),
	  "invalid code sequence reading #" );
  push(x);
  require("read",
	  get_token(ios) == tok_rbrack,
	  "invalid closure literal reading #" );
  closure_s( &Sref(3), &Sref(2), &Sref(1) );

  return (token_value = pop());
}

static value_t read_binary( FILE *ios ) {
  Ctype_t ctype = token_Ctype; size_t n = 0;
  value_t x;
  size_t elsize = Ctype_size(ctype);
  token_t tok;

  /* TODO: validate the symbol name further */
  index_t base = Sp;
  
  while ((tok = get_token( ios )) != tok_rbrack ) {
    require( "read",
	     tok != tok_eof,
	     "unexpected EOF reading #" );

    x = read_expr( ios );
    push(x);
    n++;
  }

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

  out = read_expr( ios );
  out = list2( macro_sym, out );
  return (token_value = out);
}

value_t read_expr( FILE *ios ) {
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

  case tok_quote:
    read_macro( ios, tok );
    out = take( ios );
    break;

  case tok_symbol ... tok_string:
    out = take( ios );
    break;
    
  case tok_binary:
    take( ios );
    read_binary( ios );
    out = take( ios );
    break;

  case tok_function:
    take( ios );
    read_function( ios );
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

value_t r_read( FILE *ios ) { // API for read_expr
  assert( token == tok_ready );

  clear_reader();

  value_t out = read_expr( ios );

  return out;
}

value_t r_load( char *fname ) {
  assert( token == tok_ready );
  assert( fname );

  require( "load",
	   is_rascal_file(fname),
	   "unrecognized extension in '%s'",
	   fname );

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

  else if (is_rascal_data_file(fname)) {
    size_t n = 0;

    while ((token = get_token(ios)) != tok_eof) {
      value_t xpr = read_expr(ios);
      push(xpr);
      n++;
    }

    out = vector_s( n, Stack+Sp-n );

  } else if (is_rascal_object_file(fname)) {
    value_t envt = read_expr(ios);
    assert(!feof(ios));
    push(envt);
    value_t vals = read_expr(ios);
    assert(!feof(ios));
    push(vals);
    value_t code = read_expr(ios);
    push(code);
    assert(feof(ios));
    closure_s(&Sref(3), &Sref(2), &Sref(1));
    out = pop();
  }

  else {
    while ((token = get_token(ios)) != tok_eof) {
      value_t xpr  = read_expr( ios );
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
  assert( token == tok_ready );
  require( "comp-file",
	   is_rascal_source_file(fname),
	   "unrecognized file extension in '%s'" );

  char *ext = strstr(fname, ".rsp" );

  char outfile_name[ext - fname + 7];
  strncpy( outfile_name, fname, ext - fname );
  strcpy( outfile_name + (ext-fname), ".rdn.o" );

  FILE *ios = fopen( fname, "rt" );
  value_t out = val_nil;

  require( "comp-file",
	   ios,
	   strerror( errno ) );

  clear_reader();

  int saveSp = Sp, saveFp = Fp, savePc = Pc, saveBp = Bp;

  if (setjmp(Toplevel)) {
    fprintf( stderr, "aborting to toplevel.\n" );
    out = val_nil;
  }

  else {
    push( r_do ); // compile as a squence expression   
    while ((token = get_token(ios)) != tok_eof) {
      value_t xpr  = read_expr( ios );
      push(xpr);
    }
    value_t module = list( Sp - saveSp, &Stack[saveSp] );
    value_t compiled_module = compile( module );

    // create object file
    FILE *outfile = fopen(outfile_name, "wt+" );

    fprintf(outfile, ";;; Environment:\n\n" );
    r_prin( outfile, clenvt(compiled_module) );
    fprintf(outfile, "\n\n;;; Stored Values:\n\n" );
    r_prin(outfile, clvals(compiled_module) );
    fprintf(outfile, "\n\n;;; Code:\n\n" );
    r_prin(outfile, clcode(compiled_module) );

    fflush( outfile );
    fclose( outfile );
  }

  Sp = saveSp;
  Fp = saveFp;
  Pc = savePc;
  Bp = saveBp;

  clear_reader();
  fclose( ios );

  return out;
}

void r_repl( void ) {
  for (;;) {
    index_t saveSp = Sp, saveFp = Fp, savePc = Pc, saveBp = Bp;
    clear_reader();

    if (setjmp(Toplevel)) {
      printf( "recovering.\n" );
      
    } else {
      printf( prompt_in );
      value_t xpr = r_read( stdin );
      printf( "\n" );
      xpr = eval( xpr );
      printf( prompt_out );
      r_prin( stdin, xpr );
      printf( "\n" );
    }

    Sp = saveSp;
    Fp = saveFp;
    Pc = savePc;
    Bp = saveBp;
  }
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
  argc( "load", n, 1 );
  argt( "load", Tos, type_string );
  value_t val = r_load( adata(Tos) );
  Tos = val;
}

void r_builtin(comp_file) {
  argc("comp-file", n, 1);
  argt( "load", Tos, type_string );
  value_t val = r_comp_file( adata(Tos) );
  Tos = val;
}

void r_builtin(repl) {
  argc( "repl", n, 0);
  r_repl();
}

// initialization -------------------------------------------------------------
void init_io( void ) {
  // create module builtins
  builtin( "read", builtin_read );
  builtin( "prin", builtin_prin );
  builtin( "load", builtin_load );
  builtin( "comp-file", builtin_comp_file );
  builtin( "repl", builtin_repl );
}
