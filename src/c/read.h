#ifndef read_h
#define read_h

#include <stdio.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  READY_TOKEN,
  EXPRESSION_TOKEN,
  EOF_TOKEN,
  ERROR_TOKEN,
  INIT_TOKEN
} token_t;

struct Reader {
  table_t dispatch;
  vector_t expressions;
  binary_t buffer;
  value_t expression;
  FILE* infile;
  token_t state;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern struct Reader Reader;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( FILE* ios );
void reset_reader( struct Reader* reader );
void init_reader( struct Reader* reader, FILE* infile );
void reader_init( void );

#endif
