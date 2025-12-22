#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"
#include "data.h"
#include "lang.h"
#include "collection.h"

// magic numbers
#define HEAPLOAD 0.75

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
  .arity  = 0,
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
  .nosweep = true
};

Port Outs = {
  .type    = &PortType,
  .black   = false,
  .gray    = true,
  .nosweep = true  
};

Port Errs = {
  .type    = &PortType,
  .black   = false,
  .gray    = true,
  .nosweep = true
};

// mostly whitespace and control characters
char* CharNames[128] = {
  ['\0'] = "nul",      ['\n'] = "newline",
  [' ']  = "space",    ['\a'] = "bel",
  ['\t'] = "tab",      ['\r'] = "return",
  ['\f'] = "formfeed", ['\v'] = "vtab",
  ['\b'] = "backspace"
};

// internal helpers
static bool check_gc(RlState* rls, size_t n);
static void mark_vm(RlState* rls);
static void mark_globals(RlState* rls);
static void mark_upvals(RlState* rls);
static void setup_phase(RlState* rls);
static void mark_phase(RlState* rls);
static void trace_phase(RlState* rls);
static void sweep_phase(RlState* rls);
static void cleanup_phase(RlState* rls);

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
  pprintf(&Errs, "%s error: ", ErrorNames[etype]);
  pvprintf(&Errs, fmt, va);
  pprintf(&Errs, ".\n");
  va_end(va);

#ifdef RASCAL_DEBUG
  // stack_report(&Main);
#endif

  if ( etype == SYSTEM_ERROR )
    exit(1);

  else if ( rls->ep == 0 ) {
    fprintf(stderr, "Exiting because control reached toplevel.\n");
    exit(1);
  }

  ErrorState* state = error_state(rls);
  longjmp(state->Cstate, 1);
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

void save_frame(RlState* rls) {
  if ( rls->fp == CALL_STACK_SIZE )
    runtime_error(rls, "runtime:save_frame", "stack overflow");


  CallState *frame = &rls->frames[rls->fp++];
  frame->frame_size = 1 + rls->sp - rls->bp;
  frame->cntl_off = -1; // not used until effects are implemented
  frame->flags = -1; // not used until effects are implemented
  frame->savepc = rls->pc;
  frame->savefn = rls->fn;
}

void restore_frame(RlState* rls) {
  assert(rls->fp > 0);
  CallState* frame = &rls->frames[--rls->fp];
  rls->pc = frame->savepc;
  rls->fn = frame->savefn;
  rls->bp = rls->sp - frame->frame_size + 1;
}

void reset_vm(RlState* rls) {
  rls->upvs = NULL;
  rls->pc = NULL;
  rls->fn = NULL;
  rls->fp =  0;
  rls->bp =  0;
  rls->sp =  0;
}

// garbage collector
static bool check_gc(RlState* rls, size_t n) {
  return rls->vm->heap_used + n >= rls->vm->heap_cap;
}

static bool check_heap_grow(RlState* rls) {
  return rls->vm->heap_cap * HEAPLOAD < rls->vm->heap_used;
}

static void mark_vm(RlState* rls) {
  mark_obj(rls, rls->fn);

  for ( int i=0; i < rls->sp; i++ )
    mark_exp(rls, rls->stack[i]);

  // Mark saved methods in call frames
  for ( int i=0; i < rls->fp; i++ )
    mark_obj(rls, rls->frames[i].savefn);
}

static void mark_globals(RlState* rls) {
  Obj* permanent = rls->vm->permanent_objects;

  while ( permanent != NULL ) {
    mark_obj(rls, permanent);
    permanent = permanent->heap;
  }
}

static void mark_upvals(RlState* rls) {
  UpVal* upv = rls->upvs;

  while ( upv != NULL ) {
    mark_obj(rls, upv);
    upv = upv->next;
  }
}

static void setup_phase(RlState* rls) {
  rls->vm->gc = true;
}

static void mark_phase(RlState* rls) {
  mark_vm(rls);
  mark_globals(rls);
  mark_upvals(rls);
}

static void trace_phase(RlState* rls) {
  while ( rls->vm->grays->count > 0 ) {
    Obj* obj       = objs_pop(rls, rls->vm->grays);
    Type* info     = obj->type;
    obj->gray      = false;

    info->trace_fn(rls, obj);
  }
}

static void sweep_phase(RlState* rls) {
  Obj* obj = rls->vm->permanent_objects;

  while ( obj != NULL ) { // permanent objects only need to have their GC bits reset
    obj->black = false;
    obj->gray = true;
    obj = obj->heap;
  }
  
  Obj** spc = &rls->vm->managed_objects;

  while ( *spc != NULL ) {
    obj = *spc;

    if ( obj->black ) {         // preserve
      obj->black = false;
      obj->gray  = true;
      spc        = &obj->heap;
    } else {                    // reclaim
      *spc = obj->heap;

      free_obj(rls, obj);
    }
  }
}

static void cleanup_phase(RlState* rls) {
  if ( check_heap_grow(rls) )
    rls->vm->heap_cap <<= 1;

  rls->vm->gc = false;
  rls->vm->gc_count++;
}

void add_to_managed(RlState* rls, void* ptr) {
  Obj* obj  = ptr;
  obj->heap = rls->vm->managed_objects;
  rls->vm->managed_objects = obj;
}

void add_to_permanent(RlState* rls, void* ptr) {
  Obj* obj = ptr;
  obj->heap = rls->vm->permanent_objects;
  rls->vm->permanent_objects = obj;
}

void gc_save(RlState* rls, void* ob) {
  objs_push(rls, rls->vm->grays, ob);
}

void run_gc(RlState* rls) {
#ifdef RASCAL_DEBUG
  printf("\n\nINFO: entering gc.\n\n");
  heap_report(rls);
  env_report(rls);
#endif

  if ( rls->vm->initialized ) {
    setup_phase(rls);
    mark_phase(rls);
    trace_phase(rls);
    sweep_phase(rls);
    cleanup_phase(rls);
  } else {
    rls->vm->heap_cap <<= 1;
  }

#ifdef RASCAL_DEBUG
  printf("\n\nINFO: exiting gc.\n\n");
  heap_report(rls);
  env_report(rls);
#endif

}

void heap_report(RlState* rls) {
  printf("\n\n==== heap report ====\n\%-16s %20zu\n%-16s %20zu\n%-16s %20zu\n\n",
         "used",
         rls->vm->heap_used,
         "available",
         rls->vm->heap_cap,
         "gc count",
         rls->vm->gc_count);
}

void stack_report_slice(RlState* rls, char* msg, int n) {
  printf("\n\n=== %s ===\n\n", msg);
  Expr* base = &rls->stack[rls->sp-n];

  for ( int i=n-1; i >= 0; i-- ) {
    printf("%4d", i);
    print_exp(&Outs, rls->stack[i]);
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

void* allocate(RlState* rls, size_t n) {
  void* out;
  bool h = rls != NULL;

  if ( h ) {
    if ( check_gc(rls, n) )
      run_gc(rls);

    out = calloc(n, 1);
    rls->vm->heap_used += n;
  } else
    out = calloc(n, 1);

  if ( out == NULL )
    system_error(rls, "out of memory");

  return out;
}

char* duplicates(RlState* rls, char* cs) {
  (void)rls; // for consistency
  char* out = strdup(cs);

  if ( out == NULL )
    system_error(rls, "out of memory");

  return out;
}

void* duplicate(RlState* rls, size_t n, void* ptr) {
  void* spc = allocate(rls, n);

  memcpy(spc, ptr, n);

  return spc;
}

void* reallocate(RlState* rls, size_t n, size_t o, void* spc) {
  void* out;
  bool h = rls != NULL;

  if ( n == 0 ) {
    out = NULL;

    release(rls, spc, o * h);
  } else if ( o == 0 ) {
    out = allocate(rls, n);
  } else {
    if ( h ) {
      if ( o > n ) {
        size_t diff = o - n;
        out = realloc(spc, n);

        if ( out == NULL )
          system_error(rls, "out of memory");

        rls->vm->heap_used -= diff;
      } else if ( o < n ) {
        size_t diff = n - o;

        if ( check_gc(rls, diff) )
          run_gc(rls);

        out = realloc(spc, n);

        if ( out == NULL )
          system_error(rls, "out of memory");

        memset(out+o, 0, diff);

        rls->vm->heap_used += diff;
      }
    } else {
      out = realloc(spc, n);

      if ( out == NULL )
        system_error(rls, "out of memory");

      if ( o < n )
        memset(out+o, 0, n-o);
    }
  }

  return out;
}

void release(RlState* rls, void* spc, size_t n) {
  free(spc);

  if ( rls != NULL )
    rls->vm->heap_used -= n;
}
