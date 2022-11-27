#ifndef rl_vm_reader_h
#define rl_vm_reader_h

#include "rascal.h"

/* commentary */

/* C types */
typedef enum token_t
  {
   token_ready=0,
   token_expr =1,
   token_eos  =2,
   token_error=3,
  } token_t;

struct reader_t
{
  stream_t stream;
  buffer_t buffer;
  token_t  token;
  value_t  value;
};

/* globals */
extern reader_t Reader;

/* API */
void reset_reader( reader_t *reader, stream_t stream );
void clear_reader( reader_t *reader );

/* runtime */
void rl_vm_reader_init( void );

/* convenience */

#endif
