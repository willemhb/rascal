#ifndef rascal_io_h
#define rascal_io_h

#include "rascal.h"

typedef enum {
  tok_lpar,
  tok_rpar,

  tok_lbrack,
  tok_rbrack,

  tok_hash,
  tok_quote,
  tok_dot,

  tok_symbol,
  tok_integer,
  tok_true,
  tok_false,
  tok_nil,
  tok_character,
  tok_string,

  tok_ready,
  tok_eof
} token_t;

// core -----------------------------------------------------------------------
size_t  r_prin( FILE *ios, value_t x );
value_t r_read( FILE *ios );
value_t r_load( char *fname );
value_t r_comp_file(char *fname );

void init_io( void );

#endif
