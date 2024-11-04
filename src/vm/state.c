#include "vm/state.h"

#include "val/array.h"
#include "val/table.h"

/* Globals */
/* Stacks */
Val    Stack[MAX_STACK];
EFrame Errors[MAX_ERROR];

/* Global array objects */
Alist Grays = {
  .tag   = T_ALIST,
  .free  = true,
  .gray  = true,
  .cnt   = 0,
  .cap   = 0,
  .data  = NULL
};

/* Global table objects */
Table Globals = {
  .tag   = T_TABLE,
  .trace = true,
  .free  = true,
  .gray  = true,
  .cnt   = 0,
  .cap   = 0,
  .kvs   = NULL,
  .map   = NULL
};

/* Global State object */
State Vm = {};

/* APIs for State object. */
void rl_init_state(State* state, Alist* grays, Table* globals, Val* stack, EFrame* errors) {
  /* Initialize heap state */
  state->hfs     = NULL;
  state->grays   = grays;
  state->objs    = NULL;
  state->alloc   = 0;
  state->limit   = INIT_HEAP;

  /* Initialize error state */
  state->ebase   = errors;
  state->ep      = errors;
  state->elast   = errors + MAX_ERROR;

  /* Initialize environment state */
  state->upvals  = NULL;
  state->globals = globals;
  state->module  = globals;
  state->symbols = NULL;

  /* Initialize execution state */
  state->code    = NULL;
  state->ip      = NULL;
  state->fp      = NULL;
  state->bp      = NULL;

  /* Initialize stack state */
  state->sbase   = stack;
  state->sp      = stack;
  state->slast   = stack + MAX_STACK;
}

void rl_toplevel_init_state(void) {
  rl_init_state(&Vm, &Grays, &Globals, Stack, Errors);
}
