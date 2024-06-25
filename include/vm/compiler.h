#ifndef rl_vm_compiler_h
#define rl_vm_compiler_h

#include "val/array.h"

/* C types */
struct CState {
  Alist frames;
  MVec  stack;
  List* form;
  Env*  envt;
  MVec* constants;
  MBin* code;
};

/* Globals */
extern CState Compiler;

/* External API */

#endif
