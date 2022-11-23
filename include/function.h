#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"

struct lambda_t
{
  OBJHEAD;

  vector_t       *constants;   // constant store
  environment_t  *lenvt;       // lexical environment
  instructions_t *code;        // instruction sequence
};

/* implementaiton and utilities */
lambda_t *make_lambda( void );
void      init_lambda( lambda_t *lambda, environment_t *environment );
void      free_lambda( lambda_t *lambda );


#endif
