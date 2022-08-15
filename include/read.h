#ifndef rascal_read_h
#define rascal_read_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef enum {
  TOK_LPAR,
  TOK_RPAR,
  TOK_LBRACK,
  TOK_RBRACK,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_HASH,
  TOK_QUOTE,
  TOK_DOT,
  TOK_BINARY,
  TOK_STRING,
  TOK_INTEGER,
  TOK_DOUBLE,
  TOK_CHARACTER,
  TOK_BOOLEAN,
  TOK_SYMBOL
} token_t;

// utilities ------------------------------------------------------------------
size_t  rsp_read(port_t *ios);
value_t rsp_load(char_t *fname);

// native functions -----------------------------------------------------------
value_t native_read(value_t *args, size_t n_args);
value_t native_load(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void read_init(void);

#endif
