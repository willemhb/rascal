#ifndef rascal_function_h
#define rascal_function_h

#include "obj/type.h"

typedef struct
{
  OBJECT

  object_t *name;
  object_t *type;     // the type this function is a constructor for (if any)
  object_t *methods;
} function_t;

typedef struct
{
  OBJECT

  object_t *cache;   // cache link (methods)
  object_t *table;   // 
  object_t *method;
} methods_t;

typedef struct
{
  OBJECT
  value_t   invoke;

  bool      vargs;
  arity_t   nargs;
  object_t *signature;
} method_t;

// globals
extern type_t FunctionType, MethodsType, MethodType;

#endif
