#ifndef rl_vm_compiler_h
#define rl_vm_compiler_h

#include "val/array.h"

/* C types */
typedef enum CFlag {
  COMPILE_TOPLEVEL,
  COMPILE_SCRIPT,
  COMPILE_NAMESPACE,
  COMPILE_CLOSURE,
  COMPILE_MACRO
} CFlag;

struct CState {
  MVec   stack;
  CFlag  flag;
  bool   tail;
  List*  form;
  Env*   envt;
  MVec*  vals;
  MBin*  code;
};

/* Globals */
extern CState Compiler;

/* External APIs */

#endif
