#include "val/ctl.h"
#include "vm.h"

// C types --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
void trace_ctl(RlState* rls, void* ptr);

// Globals --------------------------------------------------------------------
Type CtlType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_CTL,
  .obsize   = sizeof(Ctl),
  .trace_fn = trace_ctl
};

// Implementations ------------------------------------------------------------
Ctl* mk_ctl(RlState* rls, Fun* handler) {
  Ctl* out = mk_obj(rls, &CtlType, 0);
  out->handler = handler;
  // actual stack state initialized elsewhere
  return out;
}

Ctl* mk_ctl_s(RlState* rls, Fun* handler) {
  Ctl* out = mk_ctl(rls, handler);
  stack_push(rls, tag_obj(out));
  return out;
}

void init_ctl(RlState* rls, Ctl* ctl) {
  // initialize the fields of ctl with execution state
  ctl->exec = rls->exec;
  ctl->pc = rls->pc;
  ctl->s_top = rls->s_top;
  ctl->base = rls->base;
  ctl->f_top = rls->f_top;
  ctl->esc = rls->esc;
}

void trace_ctl(RlState* rls, void* ptr) {
  Ctl* ctl = ptr;

  // strictly speaking ctl->exec is on the stack and doesn't need to be traced,
  // but since I'm planning on eventually porting the Ctl type to a resumable
  // continuation this is just futureproofing
  mark_obj(rls, ctl->exec);
  mark_obj(rls, ctl->handler);
}
