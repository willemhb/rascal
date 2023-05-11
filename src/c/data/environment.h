#ifndef data_environment_h
#define data_environment_h

#include "data/object.h"

// C types
struct environment {
  HEADER;
  environment_t* parent;
  table_t*       locals;
  table_t*       upvalues;
};

// API & utilities
#define is_environment(x) (value_type(x) == ENVIRONMENT)
#define as_environment(x) ((environment_t*)(((value_t)(x))&WVMASK))

environment_t* make_environment(environment_t* parent);

bool define(value_t name, environment_t* environment);
int  lookup(value_t name, environment_t* environment);

#endif
