#ifndef rascal_io_h
#define rascal_io_h

#include "rascal.h"

typedef enum {
  tok_lpar,
  tok_rpar,

  tok_lbrack,
  tok_rbrack,

  tok_quote,
  tok_dot,

  tok_symbol,
  tok_integer,
  tok_true,
  tok_false,
  tok_nil,
  tok_character,
  tok_string,

  tok_function,
  tok_binary,

  tok_ready,
  tok_eof
} token_t;

// core -----------------------------------------------------------------------
size_t  r_prin( FILE *ios, value_t x );
value_t r_read( FILE *ios );
value_t r_load( char *fname );
void    r_repl( void );
value_t r_comp_file(char *fname );

// utilities ------------------------------------------------------------------
bool is_rascal_source_file( const char *fname );
bool is_rascal_data_file( const char *fname );
bool is_rascal_object_file( const char *fname );
bool is_rascal_file( const char *fname );


// builtins -------------------------------------------------------------------
void r_builtin(prin);
void r_builtin(read);
void r_builtin(load);
void r_builtin(repl);
void r_builtin(comp_file);

// initialization -------------------------------------------------------------
void io_init( void );

#endif
