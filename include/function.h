#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"

struct lambda_t
{
  OBJHEAD;

  vector_t       *constants;
  environment_t  *lenvt;
  instructions_t *code;
};

// implementations

#endif
