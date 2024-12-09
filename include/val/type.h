#ifndef rl_val_type_h
#define rl_val_type_h

#include "val/object.h"

/*
 * First-class type representation. And supporting types.
 * 
 * 
 */

/* C types */
struct Type {
  HEADER;

  union {
    char* _name;
    Sym*  name;
  };

  size64  id;     // for builtin types this is the same as the vtype
  VTable* vtable; // runtime information and methods
  Map*    proto;  // for record types this is the map object to clone when creating a new instance
  Func*   ctor;
};

struct VTable {
  VType   vtype;
  size16  dsize, obsize;
  Val     tag;
  TraceFn trace;
  FreeFn  free;
  CloneFn clone;
  SealFn  seal;
  BoxFn   box;
  UnboxFn unbox;
  HashFn  hash;
  EgalFn  egal;
  OrderFn order;
};

/* Globals */
extern Type TypeType, AnyType, NoneType;

/* API */
#define is_type(x) has_vtype(x, T_TYPE)
#define as_type(x) ((Type*)as_obj(x))

void init_builtin_type(State* vm, Type* t);

/* Initialization */
void rl_init_val_type(void);

#endif
