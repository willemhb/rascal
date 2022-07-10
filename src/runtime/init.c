#include <string.h>
#include <limits.h>

#include "memutils.h"
#include "hashing.h"

#include "runtime/init.h"
#include "instructions.h"
#include "runtime/error.h"
#include "runtime/table.h"
#include "runtime/memory.h"
#include "runtime/object.h"

#define N_SYMNODES  512
#define N_SYTXNODES 512
#define N_CHARS     CHAR_MAX+1
#define MAX_CHAR    121

#define streq( s1, s2 ) (strcmp(s1,s2)==0)

value_t readchar( char *token ) {
  static const char *charnames[N_CHARS][10] = {
    ['a'] = { "ack" },
    ['b'] = { "backspace", "bel" }, 
    ['c'] = { "can" },
    ['d'] = { "dc1", "dc2", "dc3", "dc4", "del", "dle" },
    ['e'] = { "em", "enq", "eot", "escape", "etb", "etx" },
    ['f'] = { "formfeed", "fs" },
    ['g'] = { "gs" },
    ['n'] = { "nak", "newline", "nul" },
    ['r'] = { "return", "rs" },
    ['s'] = { "si", "soh", "space", "ss", "stx", "sub", "syn" },
    ['t'] = { "tab" },
    ['u'] = { "us" },
    ['v'] = { "vtab" }
  };

  static const char charassoc[N_CHARS][10] = {
    ['a'] = { '\x06' },
    ['b'] = { '\b', '\a' },
    ['c'] = { '\x18' },
    ['d'] = { '\x11', '\x12', '\x13', '\x14', '\x7f', '\x10' },
    ['f'] = { '\f', '\x1c' },
    ['g'] = { '\x1d' },
    ['e'] = { '\x19', '\x05', '\x04', '\e', '\x17', '\x03' },
    ['n'] = { '\x15', '\n', '\0' },
    ['r'] = { '\r', '\x1e' },
    ['s'] = { '\x0f', '\x01', ' ', '\x0e', '\x02', '\x1a', '\x16' },
    ['t'] = { '\t' },
    ['u'] = { '\x1f' },
    ['v'] = { '\v' }
  };

  if ( strlen( token ) == 1 )
    return mk_character( *token );

  char *names[10]  = charnames[*token];
  char  values[10] = charassoc[*token];

  for (int i=0; i<10 && names[i]; i++) {
    int o = strcmp( token, names[i] );

    if ( o == 0 )
      return mk_character( values[i] );

    if ( o > 0 )
      break;
  }

  error( "unrecognized character %s", token );
  return rnull;
}

size_t prin_char( FILE *ios, value_t x ) {  
  static const char* charnames[N_CHARS] = {
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
}
// exports --------------------------------------------------------------------
void init_memory( void ) {
  /* Initialize global memory stores and VM state. */
  
  // allocate space for heaps
  Heap    = malloc_s( HeapSize );
  Reserve = malloc_s( RSize );
  Free    = Heap;

  // clean stacks
  memset( Stack, 0, N_STACK * sizeof( value_t ) );
  memset( Dump,  0, N_STACK * sizeof( value_t ) );
}

void init_tables( void ) {
  /* initialize global tables */
  // initialize the symbol table
  Symbols.type      = type_table;
  obflags(&Symbols) = memfl_static|memfl_global;
  Symbols.len       = 0;
  Symbols.cap       = N_SYMNODES;

  Symbols.data      = malloc_s( N_SYMNODES * sizeof(node_t) );

  // initialize syntax table
  Syntax.type       = type_table;
  obflags(&Syntax)  = memfl_static|memfl_global;
  Syntax.len        = 0;
  Syntax.cap        = N_SYTXNODES;

  Syntax.data  = malloc_s( N_SYTXNODES * sizeof(node_t) );

  // initialize characters table
  Characters.type      = type_table;
  obflags(&Characters) = memfl_static|memfl_global;
  Characters.len       = 0;
  Characters.cap       = N_CHARS;

  Characters.data      = malloc_s( N_CHARS * sizeof(node_t) );

  // intern characters
  for (int ch = '\0'; ch < CHAR_MAX; ch++) {
    char  *s = (char*)charnames[ch];
    int    n = strlen( s );
    hash_t h = strhash( s );
    intern( &Characters, s, n, h, do_intern_character );
  }
}
