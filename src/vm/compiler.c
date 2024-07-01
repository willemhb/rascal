#include "vm/compiler.h"


/* Globals */
Val CSVals[MAX_STACK];

CState Compiler = {
  .stk = {
    .type  = &MVecType,
    .trace =true,
    .sweep =false,
    .gray  =true,

    .algo  =RESIZE_PYTHON,
    .shrink=false,

    .data   =CSVals,
    ._static=CSVals,
    .cnt    =0,
    .maxc   =MAX_STACK,
    .maxs   =MAX_STACK
  },

  .flag = COMPILE_READY,
  .tail = false,
  .err  = OKAY,
  .base = -1,
  .form = NULL,
  .envt = NULL,
  .vals = NULL,
  .code = NULL,
};

/* External APIs */
void reset_cstate(CState* s) {
  free_mvec(&s->stk);

  s->flag = COMPILE_READY;
  s->tail = false;
  s->err  = OKAY;
  s->base = -1;
  s->name = NULL;
  s->form = NULL;
  s->envt = NULL;
  s->vals = NULL;
  s->code = NULL;
}

void cs_push_flag(CState* s) {
  mvec_push(&s->stk, tag((int)s->flag));
}

void cs_push_tail(CState* s) {
  mvec_push(&s->stk, tag(s->tail));
}

void cs_push_base(CState* s) {
  mvec_push(&s->stk, tag(s->base));
}

void cs_push_name(CState* s) {
  mvec_push(&s->stk, tag(s->name));
}

void cs_push_form(CState* s) {
  mvec_push(&s->stk, tag(s->form));
}

void cs_push_envt(CState* s) {
  mvec_push(&s->stk, tag(s->envt));
}

void cs_push_vals(CState* s) {
  mvec_push(&s->stk, tag(s->vals));
}

void cs_push_code(CState* s) {
  mvec_push(&s->stk, tag(s->code));
}
