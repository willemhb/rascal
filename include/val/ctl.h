#ifndef rl_val_ctl_h
#define rl_val_ctl_h

#include "val/val.h"

// C types --------------------------------------------------------------------
/* Represents program state for error handling

 */

struct Ctl {
  HEAD;
  // handler (only executes on abnormal exit)
  Fun* handler;

  // execution state
  Method* exec;
  instr_t* pc;
  StackRef s_top, base, esc;
  FrameRef f_top;
  jmp_buf Cstate;
};

// Globals --------------------------------------------------------------------
extern Type CtlType;

// Prototypes -----------------------------------------------------------------
Ctl* mk_ctl(RlState* rls, Fun* handler);
Ctl* mk_ctl_s(RlState* rls, Fun* handler);
void init_ctl(RlState* rls, Ctl* ctl);

// Helpers/Macros -------------------------------------------------------------
#define is_ctl(x)      has_type(x, &CtlType)
#define as_ctl(x)      ((Ctl*)as_obj(x))
#define is_user_ctl(x) ((x)->handler != NULL)

#endif
