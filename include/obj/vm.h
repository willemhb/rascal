#ifndef rascal_vm_h
#define rascal_vm_h

#include "obj.h"

DECL_OBJ(vm);
DECL_OBJ_API(vm);

struct vm_t
{
  OBJ_HEAD;
  obj_t    *stack;

  // vm state ('registers')
  obj_t    *code;
  obj_t    *envt;
  opcode_t *ip;
  arity_t   bp;
  arity_t   count;

  // upvalues list
  obj_t    *open_upvals;
};

// globals --------------------------------------------------------------------
extern vm_t Vm;

#endif
