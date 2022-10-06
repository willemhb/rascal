#ifndef rascal_type_h
#define rascal_type_h

#include "func.h"
#include "tuple.h"
#include "table.h"
#include "utils.h"

struct runtime_t
{
  obj_t     obj;
  size_t    base_size;

  mark_fn_t mark;
  free_fn_t free;
};

struct slot_t
{
  obj_t obj;
  
};

struct dtype_t
{
  func_t     func;
  type_t     self;
  
  runtime_t *runtime;
};

struct utype_t
{
  func_t   func;
  tuple_t *members;
};

// globals
extern alist_t Types;

// forward declarations

// convenience

#endif
