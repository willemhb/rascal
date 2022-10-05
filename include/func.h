#ifndef rascal_func_h
#define rascal_func_h

#include "table.h"

struct func_t
{
  obj_t   obj;
  atom_t *name;
};

struct metht_t
{ 
  obj_t      obj;
  func_t    *func;
  method_t  *thunk;
};

struct mcache_t
{
  obj_t obj;
  TABLE_SLOTS(method_t*);
};

struct mlevel_t
{
  obj_t obj;
  
};

struct method_t
{
  obj_t    obj;
  func_t  *func;
  types_t *signature;
  
};

#endif
