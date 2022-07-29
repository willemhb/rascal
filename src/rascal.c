// standard headers
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>

// utilties
#include "memutils.h"
#include "numutils.h"
#include "hashing.h"

// core
#include "rascal.h"
#include "runtime.h"
#include "io.h"
#include "vm.h"
#include "object.h"

// object
#include "symbol.h"
#include "list.h"
#include "array.h"
#include "function.h"
#include "number.h"
#include "table.h"

/* globals */
// global meta-info macros
#define rascal_major_version      0
#define rascal_minor_version      0
#define rascal_revision_version   1
#define rascal_version_state      "a"

#define lisp_path "/home/willem/Programming/Clang/languages/rascal/lisp/"

// global tables
ulong Symcnt = 0;

symbols_t *Symbols = NULL;

// stacks and stack state
value_t Stack[N_STACK];
index_t Sp = 0, Fp = 0, Pc = 0, Bp = 0;

// heaps and heap state
uchar *Heap, *Reserve, *Map;

size_t  HSize, HUsed, RUsed;

bool Collecting = false, Grow = false, Grew = false; 

float Collectf = 1.0, Resizef = 0.75, Growf = 2.0;

// error handling
jmp_buf Toplevel;

// dispatch tables
char    *Typenames[type_pad] = {
  [type_fixnum]  = "fixnum",  [type_type]      = "type",
  [type_boolean] = "bool",    [type_character] = "character",
  [type_cons]    = "cons",    [type_nil]       = "nil",
  [type_builtin] = "builtin", [type_closure]   = "closure",
  [type_symbol]  = "symbol",  [type_vector]    = "vector",
  [type_binary]  = "binary",  [type_string]    = "string",
  [type_dict]    = "dict",    [type_set]       = "set"
};

size_t   (*sizeof_dispatch[type_pad])(value_t x) = {
  [type_cons]   = list_sizeof,   [type_nil]    = list_sizeof,
  [type_vector] = vector_sizeof, [type_binary] = binary_sizeof,
  [type_string] = string_sizeof
};

size_t   (*prin_dispatch[type_pad])(FILE *ios, value_t x) = {
  [type_cons]   = list_prin,     [type_nil]    = list_prin,
  [type_symbol] = symbol_prin,   [type_vector] = vector_prin,
  [type_binary] = binary_prin,   [type_string] = string_prin,
  [type_dict]   = dict_prin,     [type_set]    = set_prin
};

int (*order_dispatch[type_pad])(value_t x, value_t y) = {
  [type_cons]    = list_order,    [type_nil]    = list_order,

  [type_closure] = closure_order, [type_symbol] = symbol_order,

  [type_vector]  = vector_order,  [type_binary] = binary_order,
  [type_string]  = string_order,
  
  [type_dict]    = table_order,   [type_set]    = table_order
};

hash_t (*hash_dispatch[type_pad])( value_t x ) = {
  [type_cons]    = list_hash,     [type_nil]    = list_hash,

  [type_closure] = closure_hash,  [type_symbol] = symbol_hash,

  [type_vector]  = vector_hash,   [type_binary] = binary_hash,
  [type_string]  = string_hash,

  [type_dict]    = dict_hash,     [type_set]    = set_hash
};

value_t  empty_bins[C_float64+1] = {
  [C_sint8]  = val_ebin_s8,  [C_uint8]   = val_ebin_u8,  [C_sint16] = val_ebin_s16,
  [C_uint16] = val_ebin_u16, [C_sint32]  = val_ebin_s32, [C_uint32] = val_ebin_u32,
  [C_sint64] = val_ebin_s64, [C_float64] = val_ebin_f64
};

// global symbols and keywords
value_t r_main, r_argc, r_args;

value_t r_kw_ok, r_kw_generate, r_kw_intern;

value_t r_kw_s8, r_kw_u8, r_kw_s16, r_kw_u16, r_kw_s32, r_kw_u32, r_kw_s64, r_kw_f64;

value_t r_quote, r_if, r_lambda, r_do, r_define, r_assign, r_else;

// command line parser --------------------------------------------------------
// initialization
void rascal_init( void ) {
  // initialize core
  runtime_init();
  vm_init();
  io_init();
  object_init();

  // initialize objects
  symbol_init();
  function_init();
  list_init();
  array_init();
  number_init();
  table_init();
}

void welcome_message( void ) {
  printf( "Welcome to rascal version %d.%d.%d.%s!\n\n",
	  rascal_major_version,
	  rascal_minor_version,
	  rascal_revision_version,
	  rascal_version_state );
}

void goodbye_message( void ) {
  printf( "exiting rascal, everything okay.\n" );
}

void usage_message( void ) {
  fprintf( stderr,
	   "usage: rascal [<FILENAME>.rsp | <FILENAME>.rdn | <FILENAME>.rdn.o]"
	   " [-l load |-c compile]" );
}

void help_message( void ) {
  printf( "usage: rascal [<FILENAME>.rsp | <FILENAME>.rdn | <FILENAME>.rdn.o]"
	  " [-l load |-c compile]" );
}

int process_options( int argc, char *argv[argc] ) {
  /* command line options */
  static char short_opts_string[] = "hl:c:";

  static const struct option long_options[] = {
    { "help",    no_argument,       0, 'h' },
    { "load",    required_argument, 0, 'l' },
    { "compile", required_argument, 0, 'c' }
  };

  int c, idx=0;

  while ((c = getopt_long(argc, argv, short_opts_string, long_options, &idx)) != -1) {
    switch (idx) {
    case 'h':
      help_message();
      return 0;

    case 'l':
      r_load( optarg );
      break;

    case 'c':
      r_comp_file( optarg );
      break;

    default:
      usage_message();
      return 1;
    }
  }

  return 0;
}

int main( int argc, char *argv[argc] ) {
 /* initialize runtime */
  rascal_init();

  /* no arguments */
  if (argc == 1) {
    welcome_message();
    atexit(goodbye_message);
    r_repl();
    
    return 0;
  }

  int result = process_options( argc, argv );

  return result;
}
