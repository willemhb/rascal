#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"

struct Function
{
  OBJ_HEAD;

  String *name;

  union
  {
    Native   *native;
    Template *template;
  };
};

#endif
