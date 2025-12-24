#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm/vm.h"
#include "vm/error.h"
#include "vm/memory.h"
#include "val.h"
#include "lang.h"

// Global declarations --------------------------------------------------------
char* ErrorNames[] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [EVAL_ERROR]    = "eval",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

extern Strings StringTable;
extern Type EnvType;

Env Globals = {
  .type    = &EnvType,
  .black   = false,
  .gray    = true,
  .nosweep = true,

  .parent = NULL,
  .ncap   = 0,

  .vars = {
    .kvs       = NULL,
    .count     = 0,
    .max_count = 0
  },

  .vals = {
    .vals      = NULL,
    .count     = 0,
    .max_count = 0
  }
};

Objs GrayStack = {
  .vals      = NULL,
  .count     = 0,
  .max_count = 0
};

RlVm Vm = {
  .globals = &Globals,
  .strings = &StringTable,
  .heap_cap  = INIT_HEAP,
  .heap_used = 0,
  .gc_count  = 0,
  .gc = false,
  .initialized = false,
  .managed_objects = NULL,
  .grays = &GrayStack
};

RlState Main = {
  .vm   = &Vm,
  .upvs = NULL,
  .fn   = NULL,
  .pc   = NULL,
  .sp   = 0,
  .fp   = 0,
  .bp   = 0
};

// static wrapper objects for standard streams
extern Type PortType;

Port Ins = {
  .type    = &PortType,
  .black   = false,
  .gray    = true,
  .nosweep = true,
  .mode    = LISP_PORT | INPUT_PORT | TEXT_PORT
};

Port Outs = {
  .type    = &PortType,
  .black   = false,
  .gray    = true,
  .nosweep = true,
  .mode    = LISP_PORT | OUTPUT_PORT | TEXT_PORT
};

Port Errs = {
  .type    = &PortType,
  .black   = false,
  .gray    = true,
  .nosweep = true,
  .mode    = OUTPUT_PORT | TEXT_PORT
};

// mostly whitespace and control characters
char* CharNames[128] = {
  ['\0'] = "nul",      ['\n'] = "newline",
  [' ']  = "space",    ['\a'] = "bel",
  ['\t'] = "tab",      ['\r'] = "return",
  ['\f'] = "formfeed", ['\v'] = "vtab",
  ['\b'] = "backspace"
};

// miscellaneous state helpers
char* current_fn_name(RlState* rls) {
  char* out;

  if ( rls->fn == NULL )
    out = "&toplevel";

  else
    out = method_name(rls->fn);

  return out;
}

// token API
void reset_token(RlState* rls) {
  memset(rls->token, 0, BUFFER_SIZE);
  rls->toff = 0;
}

size_t add_to_token(RlState* rls, char c) {
  if ( rls->toff < BUFFER_MAX )
    rls->token[rls->toff++] = c;

  else
    runtime_error(rls, "maximum token length exceeded");

  return rls->toff;
}

// stack API ------------------------------------------------------------------
void reset_stack(RlState* rls) {
  memset(rls->stack, 0, EXPR_STACK_SIZE * sizeof(Expr));
  rls->sp = 0;
}

void check_stack_limit(RlState* rls, int n) {
  require(rls,
          rls->sp+n < EXPR_STACK_SIZE,
          "stack overflow in %s",
          current_fn_name(rls));
}

int check_stack_bounds(RlState* rls, int n) {
  if ( n < 0 )
    n += rls->sp;

  require(rls,
          n >= 0 && n < rls->sp,
          "stack reference %d out of bounds in %s",
          n,
          current_fn_name(rls));

  return n;
}

Expr* stack_ref(RlState* rls, int i) {
  int j = i;

  if ( j < 0 )
    j += rls->sp;

  if ( j < 0 || j > rls->sp ) {
    runtime_error(rls, "stack reference %d out of bounds", i);
  }

  return &rls->stack[j];
}

int save_sp(RlState* rls) {
  return rls->sp;
}

void restore_sp(RlState* rls, int sp) {
  // intended for restoring an sp previously recorded with `save_sp`
  assert(sp >= 0 && sp <= EXPR_STACK_SIZE);
  rls->sp = sp;
}

Expr* preserve(RlState* rls, int n, ...) {
  check_stack_limit(rls, n);
  Expr* out = &rls->stack[rls->sp];
  va_list va;
  va_start(va, n);
  for ( int i=0; i<n; i++ )
    rls->stack[rls->sp++] = va_arg(va, Expr);

  va_end(va);
  return out;
}

Expr* dup(RlState* rls) {
  // duplicate top of stack
  if ( rls->sp == 0 )
    runtime_error(rls, "stack underflow");

  rls->sp++;
  rls->stack[rls->sp-1] = rls->stack[rls->sp-2];

  return &rls->stack[rls->sp - 1];
}

Expr* push(RlState* rls, Expr x) {
  if ( rls->sp == EXPR_STACK_SIZE )
    runtime_error(rls, "stack overflow");

  rls->stack[rls->sp] = x;

  return &rls->stack[rls->sp++];
}

Expr* pushn(RlState* rls, int n) {
  if ( rls->sp + n >= EXPR_STACK_SIZE )
    runtime_error(rls, "stack overflow");

  Expr* base = &rls->stack[rls->sp]; rls->sp += n;

  return base;
}

Expr pop(RlState* rls) {
  if ( rls->sp == 0 )
    runtime_error(rls, "stack underflow");

  return rls->stack[--rls->sp];
}

Expr rpop(RlState* rls) {
  if ( rls->sp < 2 )
    runtime_error(rls, "stack underflow");

  Expr out            = rls->stack[rls->sp-2];
  rls->stack[rls->sp-2] = rls->stack[rls->sp-1];

  rls->sp--;

  return out;
}

Expr popn(RlState* rls, int n) {
  if ( n > rls->sp )
    runtime_error(rls, "stack underflow");

  Expr out = rls->stack[rls->sp-1]; rls->sp -= n;

  return out;
}

// return an UpVal object corresponding to the given stack location
UpVal* get_upv(RlState* rls, Expr* loc) {
  UpVal** spc = &rls->upvs;
  UpVal* out = NULL;

  while ( out == NULL ) {
    UpVal* upv = *spc;

    if ( upv == NULL || loc < upv->loc )
      *spc = out = mk_upval(rls, upv, loc);

    else if ( upv->loc == loc )
      out = upv;

    else
      spc = &upv->next;
  }

  return out;
}

void close_upvs(RlState* rls, Expr* base) {
  UpVal* upv = rls->upvs;

  while ( upv != NULL && upv->loc > base ) {
    UpVal* tmpu = upv->next;
    Expr tmpv   = *upv->loc;
    upv->next   = NULL;
    upv->closed = true;
    upv->val    = tmpv;
    upv         = tmpu;
  }
}

// frame manipulation
void install_method(RlState* rls, Method* method, int argc) {
  rls->fn = method;
  rls->pc = method->chunk->code->binary.vals;
  rls->bp = rls->sp-argc;
}

bool save_call_frame(RlState* rls, int argc) {
  bool out;

  if ( rls->fp == CALL_STACK_SIZE )
    runtime_error(rls, "runtime:save_frame", "stack overflow");

  if ( rls->fn == NULL )
    out = false;

  else {
    CallState *frame = &rls->frames[rls->fp++];
    frame->frame_size = (rls->sp-argc-1) - rls->bp;
    frame->cntl_off = -1; // not used until effects are implemented
    frame->flags = -1; // not used until effects are implemented
    frame->savepc = rls->pc;
    frame->savefn = rls->fn;
    out = true;
  }

  return out;
}

bool restore_call_frame(RlState* rls) {
  bool out;

  if ( rls->fp == 0 ) { // no frames to restore
    rls->fn = NULL;     // zero out state to avoid fucked up shit
    rls->pc = NULL;
    rls->bp = 0;
    out = false;
  } else {
    CallState* frame = &rls->frames[--rls->fp];
    rls->sp = rls->bp-1; // exclude caller
    rls->pc = frame->savepc;
    rls->fn = frame->savefn;
    rls->bp = rls->sp - frame->frame_size;
    out = true;
  }

  return out;
}

void reset_vm(RlState* rls) {
  rls->upvs = NULL;
  rls->pc = NULL;
  rls->fn = NULL;
  rls->fp =  0;
  rls->bp =  0;
  rls->sp =  0;
}

// report helpers
void stack_report_slice(RlState* rls, int n, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  printf("\n\n== ");
  vprintf(fmt, va);
  printf(" ==\n\n");
  va_end(va);

  Expr* base = stack_ref(rls, -n);

  for ( int i=n-1; i >= 0; i-- ) {
    printf("%4d ", i);
    print_exp(&Outs, base[i]);
    printf("\n");
  }
}

void stack_report(RlState* rls) {
  printf("\n\n==== stack report ====\n\n");

  for ( int i=rls->sp-1; i>=0; i-- ) {
    printf("%4d ", i);
    print_exp(&Outs, rls->stack[i]);
    printf("\n");
  }
}

void env_report(RlState* rls) {
  Env* g = rls->vm->globals;
  int n = env_size(g);
  Ref** rs = (Ref**)g->vals.vals;
  printf("\n\n==== env report (size = %4d) ====\n\n", n);

  for ( int i=0; i < n; i++ ) {
    printf("%4d = %s\n", i, rs[i]->name->val->val);
  }
}
