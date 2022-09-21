#ifndef rascal_exec_h
#define rascal_exec_h

#include "obj/obj.h"

// C types --------------------------------------------------------------------
typedef struct vm_t
{
  OBJ_HEAD;
  obj_t *stack; // stores function arguments and saved caller state

  // vm state ('registers')
  obj_t  *func;
  obj_t  *envt;
  ptr_t   ip;
  arity_t bp;
  arity_t argc;

  obj_t *open_upvals;
} vm_t;

// forward declarations -------------------------------------------------------


#endif
