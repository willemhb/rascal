#ifndef rascal_function_h
#define rascal_function_h

#include "obj/type.h"

// objects used in the dispatch system go here
typedef struct function_t function_t;
typedef struct methods_t  methods_t;
typedef struct method_t   method_t;

struct function_t
{
  OBJECT

  object_t *name;
  object_t *type;     // the type this function is a constructor for (if any)
  object_t *methods;
};

struct methods_t
{
  OBJECT;

  object_t *cache;    // cache link (methods)
  object_t *table;    // complete method table
  object_t *function; // function object this method table belongs to
};

struct method_t
{
  OBJECT;

  value_t   method;  // native, primitive, or code object
  bool      vargs;
  arity_t   nargs;
  object_t *signature;
};

// globals
extern type_t FunctionType, MethodsType, MethodType;

#endif
