#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "vm/vm.h"
#include "vm/error.h"
#include "val.h"

// error helpers
ErrorState* error_state(RlState* rls) {
  assert(rls->ep > 0);
  return &rls->err_states[rls->ep-1];
}

void save_error_state(RlState* rls) {
  // The Toplevel jmp_buf will probably be preserved and used for something
  if ( rls->ep == ERROR_STACK_SIZE ) {
    fprintf(stderr, "Exiting due to error stack overflow.\n");
    exit(1);
  }

  ErrorState* err_state = &rls->err_states[rls->ep++];

  err_state->fn = rls->fn;
  err_state->pc = rls->pc;
  err_state->sp = rls->sp;
  err_state->fp = rls->fp;
  err_state->bp = rls->bp;
}

void restore_error_state(RlState* rls) {
  // close any upvalues whose frames are being abandoned
  ErrorState* err_state = error_state(rls);

  close_upvs(rls, stack_ref(rls, err_state->sp));

  // restore main registers
  rls->fn = err_state->fn;
  rls->sp = err_state->sp;
  rls->pc = err_state->pc;
  rls->fp = err_state->fp;
  rls->bp = err_state->bp;
}

void discard_error_state(RlState* rls) {
  assert(rls->ep > 0);

  rls->ep--;
}

void rascal_error(RlState* rls, Status etype, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  pprintf(&Errs, "%s error in %s: ", ErrorNames[etype], current_fn_name(rls));
  pvprintf(&Errs, fmt, va);
  pprintf(&Errs, ".\n");
  va_end(va);

#ifdef RASCAL_DEBUG
  stack_report(&Main);
#endif

  if ( etype == SYSTEM_ERROR )
    exit(1);

  else if ( rls->ep == 0 ) {
    fprintf(stderr, "Exiting because control reached toplevel.\n");
    exit(1);
  }

  ErrorState* state = error_state(rls);
  longjmp(state->Cstate, etype);
}
