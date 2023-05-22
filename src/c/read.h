#ifndef read_h
#define read_h

#include <stdio.h>

#include "common.h"
#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  READY_TOKEN,
  EXPRESSION_TOKEN,
  EOF_TOKEN,
  ERROR_TOKEN
} token_t;

extern struct {
  table_t dispatch;
  vector_t expressions;
  binary_t buffer;
  value_t expression;
  FILE* infile;
  token_t state;
} Reader;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( FILE* ios );
void init_reader( void );

#endif
