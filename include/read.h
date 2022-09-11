#ifndef rascal_read_h
#define rascal_read_h

#include "types.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   // delimiter tokens
   token_lpar,
   token_rpar,
   token_lbrack,
   token_rbrack,
   token_lbrace,
   token_rbrace,
   token_dot,

   // macro characters
   token_quote,
   token_backquote,
   token_tilde,
   token_tilde_at,

   // literal tokens
   token_real,
   token_atom,
   token_string,

   // sentinel tokens
   token_eof,
   token_ready
  } token_t;

// forward declarations -------------------------------------------------------
value_t read(port_t *ios);
value_t load(char *fname);

#endif
