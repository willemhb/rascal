#ifndef rascal_read_h
#define rascal_read_h

#include "val.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    TOK_READY,
    TOK_EOF,

    TOK_LPAR,
    TOK_RPAR,
    TOK_QUOTE,
    TOK_DOT,

    TOK_ATOM,
    TOK_REAL,
    TOK_INT,
    TOK_BOOL,
    TOK_STRING,
  } tok_t;

typedef struct buffer_t
{
  arity_t len;
  arity_t cap;

  char_t *data;
} buffer_t;

// forward declarations -------------------------------------------------------
val_t read_val( obj_t *ios );
val_t load_file( obj_t *src );
val_t comp_file( obj_t *src );

#endif
