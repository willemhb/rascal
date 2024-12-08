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

  size64  id;  // for builtin types this is the same as the vtype
  VTable* vt;  // runtime information and methods
  Map*    tpl; // for record types this is the map object to clone when creating a new instance
};

struct VTable {
  VType  vtype;
  IObj*  iobj;
  ICmp*  icmp;
};

struct IObj {
  size64  obsize;
  TraceFn trace;
  FreeFn  free;
  CloneFn clone;
  SealFn  seal;
};

struct ICmp {
  HashFn  hash;
  EgalFn  egal;
  OrderFn order;
};

/* Globals */

/* API */

/* Initialization */


#endif
