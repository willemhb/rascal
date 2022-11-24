#ifndef rascal_obj_environment_h
#define rascal_obj_environment_h

#include "obj/object.h"

/* C types */
struct environment_t
{
  OBJHEAD;

  environment_t *next;

  union
  {
    vector_t    *names;
    vector_t    *binds;
  };
};

/* globals */
extern type_t EnvironmentType;

/* API */

/* convenience & utilities */
#define is_environment( x )  value_is_type(x, &EnvironmentType)
#define as_environment( x ) ((environment_t*)as_obj(x))

#endif
