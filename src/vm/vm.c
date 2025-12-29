#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm/vm.h"
#include "vm/error.h"
#include "vm/memory.h"
#include "util/util.h"
#include "val.h"
#include "lang.h"

// Global declarations --------------------------------------------------------
Expr Stack[STACK_SIZE];
Frame CallStack[NUM_FRAMES];

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
    .maxc = 0
  },

  .vals = {
    .data = NULL,
    .count = 0,
    .maxc = 0
  }
};

RlVm Vm = {
  .globals   = &Globals,
  .strings   = {
    .kvs       = NULL,
    .count     = 0,
    .maxc = 0
  },
  .heap_cap    = INIT_HEAP,
  .heap_used   = 0,
  .gc_count    = 0,
  .gc          = false,
  .initialized = false,
  .managed_objects = NULL,
  .grays = {
    .data = NULL,
    .count = 0,
    .maxc = 0
  }
};

RlState Main = {
  .vm = &Vm,
  .upvs = NULL,
  .exec = NULL,
  .pc = NULL,
  .stack = Stack,
  .s_end = Stack+STACK_SIZE,
  .s_top = Stack,
  .base = Stack,
  .esc = NULL,
  .frames = CallStack,
  .f_end = CallStack+NUM_FRAMES,
  .f_top = CallStack
};

// static wrapper objects for standard streams
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
  .mode    = LISP_PORT | OUTPUT_PORT | TEXT_PORT
};

// token API
void reset_token(RlState* rls) {
  free_text_buf(rls, &rls->token);
}

int add_to_token(RlState* rls, char c) {
  text_buf_push(rls, &rls->token, c);

  return rls->token.count;
}

char* token_val(RlState* rls) {
  return rls->token.data;
}

int token_size(RlState* rls) {
  return rls->token.count;
}

// stack API ------------------------------------------------------------------
int stack_size(RlState* rls) {
  return rls->s_top - rls->stack;
}

int call_stack_size(RlState* rls) {
  return rls->f_top - rls->frames;
}

void stack_check_limit(RlState* rls, int n) {
  if ( n > 0 ) {
    if ( rls->s_top + n > rls->s_end )
      runtime_error(rls, "stack overflow");
  } else {
    if ( rls->s_top - n < rls->stack )
      runtime_error(rls, "stack underflow");
  }
}

int stack_check_bounds(RlState* rls, int n) {
  int ss = rls->s_top - rls->stack;
  
  if ( n < 0 ) {
    if ( rls->s_top - n < rls->stack )
      runtime_error(rls, "%d out of bounds for stack of size %d", n, ss);
  } else {
    if ( rls->stack + n > rls->s_top )
      runtime_error(rls, "%d out of bounds for stack of size %d", n, ss);
  }

  return n;
}

void frames_check_limit(RlState* rls, int n) {
  if ( n > 0 ) {
    if ( rls->f_top + n > rls->f_end )
      runtime_error(rls, "call stack overflow");
  } else {
    if ( rls->f_top - n < rls->frames )
      runtime_error(rls, "call stack underflow");
  }
}

void stack_swap(RlState* rls, int ox, int oy) {
  Expr tmp = rls->s_top[-ox];
  rls->s_top[-ox] = rls->s_top[-oy];
  rls->s_top[-oy] = tmp;
}

Expr* stack_preserve(RlState* rls, int n, ...) {
  Expr* out = rls->s_top;
  va_list va;
  va_start(va, n);
  for ( int i=0; i<n; i++ )
    *(rls->s_top++) = va_arg(va, Expr);

  va_end(va);
  return out;
}

Expr* stack_dup(RlState* rls) {
  // duplicate top of stack
  rls->s_top++;
  rls->s_top[-1] = rls->s_top[-2];

  return rls->s_top-1;
}

Expr* stack_push(RlState* rls, Expr x) {
  *rls->s_top = x;

  return rls->s_top++;
}

Expr* stack_pushn(RlState* rls, int n) {
  Expr* base = rls->s_top;
  rls->s_top += n;
  return base;
}

Expr stack_pop(RlState* rls) {
  return *(--rls->s_top);
}

Expr stack_popn(RlState* rls, int n) {
  Expr out = rls->s_top[-n];
  rls->s_top -= n;

  return out;
}

Expr stack_rpop(RlState* rls) {
  Expr out = rls->s_top[-2];
  rls->s_top[-2] = rls->s_top[-1];
  rls->s_top--;
  return out;
}

Expr stack_rpopn(RlState* rls, int n) {
  assert(n > 0);
  Expr out = rls->s_top[-n-1];
  rls->s_top[-n-1] = rls->s_top[-1];
  rls->s_top -= n;
  assert(rls->s_top > rls->stack);
  return out;
}

void stack_swap_s(RlState* rls, int ox, int oy) {
  stack_check_bounds(rls, -ox);
  stack_check_bounds(rls, -oy);
  return stack_swap(rls, ox, oy);
}

Expr* stack_preserve_s(RlState* rls, int n, ...) {
  stack_check_limit(rls, n);
  Expr* out = rls->s_top;
  va_list va;
  va_start(va, n);
  for ( int i=0; i<n; i++ )
    *(rls->s_top++) = va_arg(va, Expr);

  va_end(va);
  return out;
}

Expr* stack_dup_s(RlState* rls) {
  // duplicate top of stack
  stack_check_limit(rls, 1);

  return stack_dup(rls);
}

Expr* stack_push_s(RlState* rls, Expr x) {
  stack_check_limit(rls, 1);
  return stack_push(rls, x);
}

Expr* stack_pushn_s(RlState* rls, int n) {
  stack_check_limit(rls, n);
  return stack_pushn(rls, n);
}

Expr stack_pop_s(RlState* rls) {
  stack_check_limit(rls, -1);
  return stack_pop(rls);
}

Expr stack_popn_s(RlState* rls, int n) {
  stack_check_limit(rls, -n);
  return stack_popn(rls, n);
}

Expr stack_rpop_s(RlState* rls) {
  stack_check_limit(rls, -2);

  return stack_rpop(rls);
}

Expr stack_rpopn_s(RlState* rls, int n) {
  stack_check_limit(rls, -n-1);

  return stack_rpopn(rls, n);
}

// return an UpVal object correspondig to the given stack location
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
  rls->exec = method;
  rls->pc = method_code(method);
  rls->base = rls->s_top - argc;
}

void save_call_frame(RlState* rls) {
  assert(rls->f_top < rls->f_end);

  if ( rls->exec == NULL ) { // don't save empty call state
    assert(rls->f_top == rls->frames);
    return;
  }

  FrameRef frame = rls->f_top++;

  frame->exec = rls->exec;
  frame->pc = rls->pc;
  frame->base = rls->base;
  frame->esc = rls->esc;
}

void save_call_frame_s(RlState* rls) {
  if ( rls->f_top == rls->f_end ) {
    int count = 100;
    stack_report(rls, count, "stack state on overflow");
    runtime_error(rls, "stack overflow.");
  }

  if ( rls->exec == NULL ) { // don't save empty call state
    assert(rls->f_top == rls->frames);
    return;
  }

  FrameRef frame = rls->f_top++;

  frame->exec = rls->exec;
  frame->pc = rls->pc;
  frame->base = rls->base;
  frame->esc = rls->esc;
}

void restore_call_frame(RlState* rls) {
  assert(rls->f_top > rls->frames);

  FrameRef caller = --rls->f_top;

  rls->s_top = rls->base - 1;
  rls->exec = caller->exec;
  rls->pc = caller->pc;
  rls->esc = caller->esc;
  rls->base = caller->base;
}

void restore_call_frame_s(RlState* rls) {
  assert(rls->f_top > rls->frames);

  FrameRef caller = --rls->f_top;

  rls->s_top = rls->base - 1;
  rls->exec = caller->exec;
  rls->pc = caller->pc;
  rls->esc = caller->esc;
  rls->base = caller->base;
}

// report helpers
void stack_report(RlState* rls, int n, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  printf("\n\n=== ");
  vprintf(fmt, va);
  printf(" ===\n\n");
  va_end(va);

  if ( n == -1 )
    n = stack_size(rls);

  assert(n <= stack_size(rls));

  Expr* base = rls->s_top-n;

  for ( int i=n-1; i >= 0; i-- ) {
    printf("%4d ", i);
    print_exp(&Outs, base[i]);
    printf("\n");
  }
}

void env_report(RlState* rls, Env* vars) {
  (void)rls;
  int n = env_size(vars);

  printf("\n\n=== env report (size = %4d) ====\n\n", n);

  if ( is_global_env(vars) ) {

    Ref** rs = (Ref**)vars->vals.data;
    for ( int i=0; i < n; i++ ) {
      printf("%4d = %s\n", i, rs[i]->name->val->val);
    }
  } else {
    EMapKV* kvs = vars->vars.kvs;
    int n = vars->vars.count;
    int m = vars->vars.maxc;

    printf("\n\n=== local vars (size = %4d) ===\n\n", n);

    for ( int i=0, j=0; i < m && j < n; i++ ) {
      Ref* r = kvs[i].val;

      if ( r == NULL )
        continue;

      j++;
      printf("%4d = %s\n", r->offset, sym_val(r->name));
    }

    kvs = vars->upvs.kvs;
    n = vars->upvs.count;
    m = vars->upvs.maxc;

    printf("\n\n=== captured vars (size = %4d) ===\n\n", n);

   for ( int i=0, j=0; i < m && j < n; i++ ) {
      Ref* r = kvs[i].val;

      if ( r == NULL )
        continue;

      j++;
      printf("%4d = %s\n", r->offset, sym_val(r->name));
    }
  }
}
