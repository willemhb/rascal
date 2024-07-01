#ifndef rl_vm_compiler_h
#define rl_vm_compiler_h

#include "val/array.h"

/* C types */

struct CState {
  MVec     stk;  // pending subexpressions and saved frames
  CFlag    flag; // type of expression being compiled
  bool     tail; // indicates whether the current frame is in tail position
  rl_err_t err;  // indicates whether an error has occurred
  int      base; // offset to first subexpression
  Str*     name; // name of currently compiling function
  List*    form; // currently compiling form
  Val      head; // head of currently compiling form
  Env*     envt; // environment for current compiler frame
  MVec*    vals; // constant values for current compiler frame
  MBin*    code; // bytecode for current compiler frame
};

/* Globals */
extern CState Compiler;

/* External APIs */
void reset_cstate(CState* s);

void cs_push_flag(CState* s);
void cs_push_tail(CState* s);
void cs_push_base(CState* s);
void cs_push_name(CState* s);
void cs_push_form(CState* s);
void cs_push_head(CState* s);
void cs_push_envt(CState* s);
void cs_push_vals(CState* s);
void cs_push_code(CState* s);
void cs_push_subx(CState* s);

void cs_pop_flag(CState* s);
void cs_pop_tail(CState* s);
void cs_pop_base(CState* s);
void cs_pop_name(CState* s);
void cs_pop_form(CState* s);
void cs_pop_head(CState* s);
void cs_pop_

#endif
