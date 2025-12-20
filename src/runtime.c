#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"
#include "data.h"
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

VmCtx SaveStates[MAX_SAVESTATES];
int ep = 0;

char Token[BUFFER_SIZE];
size_t TOff = 0;
GcFrame* GcFrames = NULL;

extern Strings StringTable;

Env Globals = {
  .type    = EXP_ENV,
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
  .globals   = &Globals,
  .strings   = &StringTable,
  .heap_cap  = INIT_HEAP,
  .heap_used = 0,
  .heap_live = NULL,
  .grays     = &GrayStack
};

RlState Main = {
  .vm   = &Vm,
  .upvs = NULL,
  .fn   = NULL,
  .pc   = NULL,
  .sp   = 0,
  .fp   = 0,
  .bp   = 0,
  .ep   = 0
};

// static wrapper objects for standard streams
Port Ins = {
  .type    = EXP_PORT,
  .black   = false,
  .gray    = true,
  .nosweep = true
};

Port Outs = {
  .type    = EXP_PORT,
  .black   = false,
  .gray    = true,
  .nosweep = true  
};

Port Errs = {
  .type    = EXP_PORT,
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
static void mark_types(RlState* rls);
static void mark_gc_frames(RlState* rls);
static void mark_phase(RlState* rls);
static void trace_phase(RlState* rls);
static void sweep_phase(RlState* rls);
static void cleanup_phase(RlState* rls);

// error helpers
void panic(Status etype) {
  if ( etype == SYSTEM_ERROR )
    exit(1);

  else if ( ep == 0 ) {
    fprintf(stderr, "Exiting because control reached toplevel.\n");
    exit(1);
  }

  longjmp(SaveState.Cstate, 1);
}

void recover(void) {
  restore_ctx(&Main);
  pseek(&Ins, SEEK_SET, SEEK_END); // clear out invalid characters
}

void rascal_error(Status etype, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  pprintf(&Errs, "%s error: ", ErrorNames[etype]);
  pvprintf(&Errs, fmt, va);
  pprintf(&Errs, ".\n");
  va_end(va);
  panic(etype);
}

// token API
void reset_token(void) {
  memset(Token, 0, BUFFER_SIZE);
  TOff = 0;
}

size_t add_to_token(char c) {
  if ( TOff < BUFFER_MAX )
    Token[TOff++] = c;

  else
    runtime_error("maximum token length exceeded");

  return TOff;
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
    runtime_error("stack reference %d out of bounds", i);
  }

  return &rls->stack[j];
}

Expr* push(RlState* rls, Expr x) {
  if ( rls->sp == EXPR_STACK_SIZE )
    runtime_error("stack overflow");

  rls->stack[rls->sp] = x;

  return &rls->stack[rls->sp++];
}

Expr* pushn(RlState* rls, int n) {
  if ( rls->sp + n >= EXPR_STACK_SIZE )
    runtime_error("stack overflow");

  Expr* base = &rls->stack[rls->sp]; rls->sp += n;

  return base;
}

Expr pop(RlState* rls) {
  if ( rls->sp == 0 )
    runtime_error("stack underflow");

  return rls->stack[--rls->sp];
}

Expr rpop(RlState* rls) {
  if ( rls->sp < 2 )
    runtime_error("stack underflow");

  Expr out            = rls->stack[rls->sp-2];
  rls->stack[rls->sp-2] = rls->stack[rls->sp-1];

  rls->sp--;

  return out;
}

Expr popn(RlState* rls, int n) {
  if ( n > rls->sp )
    runtime_error("stack underflow");

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
void install_fun(RlState* rls, Fun* fun, int argc) {
  rls->fn = fun;
  rls->pc = fun->chunk->code->binary.vals;
  rls->bp = rls->sp-argc;
}

void save_frame(RlState* rls) {
  if ( rls->fp == CALL_STACK_SIZE )
    runtime_error("runtime:save_frame", "stack overflow");

  CallState *frame = &rls->frames[rls->fp++];
  frame->frame_size = rls->sp-rls->bp;
  frame->cntl_off = -1;
  frame->flags = -1;
  frame->savepc = rls->pc;
}

void restore_frame(RlState* rls) {
  assert(rls->fp > 0);
  CallState* frame = &rls->frames[--rls->fp];
  rls->pc = frame->savepc;
  rls->bp = rls->sp - frame->frame_size;
  rls->fn = as_fun(rls->stack[rls->bp-1]);
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
}

static void mark_globals(RlState* rls) {
  extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr;

  mark_obj(rls, rls->vm->globals);
  mark_obj(rls, QuoteStr);
  mark_obj(rls, DefStr);
  mark_obj(rls, PutStr);
  mark_obj(rls, IfStr);
  mark_obj(rls, DoStr);
  mark_obj(rls, FnStr);
}

static void mark_upvals(RlState* rls) {
  UpVal* upv = rls->upvs;

  while ( upv != NULL ) {
    mark_obj(rls, upv);
    upv = upv->next;
  }
}

static void mark_types(RlState* rls) {
  for ( int i=0; i < NUM_TYPES; i++ )
    mark_obj(rls, Types[i].repr);
}

static void mark_gc_frames(RlState* rls) {
  GcFrame* frame = GcFrames;

  while ( frame != NULL ) {
    for ( int i=0; i < frame->count; i++ )
      mark_exp(rls, frame->exprs[i]);

    frame = frame->next;
  }
}

static void mark_phase(RlState* rls) {
  mark_vm(rls);
  mark_globals(rls);
  mark_upvals(rls);
  mark_types(rls);
  mark_gc_frames(rls);
}

static void trace_phase(RlState* rls) {
  while ( rls->vm->grays->count > 0 ) {
    Obj* obj          = objs_pop(rls->vm->grays);
    ExpTypeInfo* info = &Types[obj->type];
    obj->gray         = false;

    info->trace_fn(rls, obj);
  }
}

static void sweep_phase(RlState* rls) {
  Obj** spc = &rls->vm->heap_live;

  while ( *spc != NULL ) {
    Obj* obj = *spc;

    if ( obj->black ) {         // preserve
      obj->black = false;
      obj->gray  = true;
      spc        = &obj->heap;
    } else {                    // reclaim
      *spc = obj->heap;

      free_obj(obj);
    }
  }
}

static void cleanup_phase(RlState* rls) {
  if ( check_heap_grow(rls) )
    rls->vm->heap_cap <<= 1;
}

void add_to_heap(RlState* rls, void* ptr) {
  Obj* obj  = ptr;
  obj->heap = rls->vm->heap_live;
  rls->vm->heap_live = obj;
}

void gc_save(RlState* rls, void* ob) {
  objs_push(rls->vm->grays, ob);
}

void run_gc(RlState* rls) {
#ifdef RASCAL_DEBUG
  printf("\n\nINFO: entering gc.\n\n");
  heap_report(rls);
#endif

  mark_phase(rls);
  trace_phase(rls);
  sweep_phase(rls);
  cleanup_phase(rls);

#ifdef RASCAL_DEBUG
  printf("\n\nINFO: exiting gc.\n\n");
  heap_report(rls);
#endif

}

void heap_report(RlState* rls) {
  printf("\n\n==== heap report ====\n\%-16s %20zu\n%-16s %20zu\n\n",
         "allocated",
         rls->vm->heap_used,
         "used",
         rls->vm->heap_cap);
}

void save_ctx(RlState* rls) {
  // The Toplevel jmp_buf will probably be preserved and used for something
  assert(ep < MAX_SAVESTATES);

  VmCtx* ctx = &SaveStates[ep++];

  ctx->fn = rls->fn;
  ctx->pc = rls->pc;
  ctx->sp = rls->sp;
  ctx->fp = rls->fp;
  ctx->bp = rls->bp;
}

void restore_ctx(RlState* rls) {
  // close any upvalues whose frames are being abandoned
  close_upvs(rls, &rls->stack[SaveState.sp]);

  // discard invalidated GC frames (important because these now point to invalid memory)
  GcFrames = SaveState.gcf;

  // restore main registers
  rls->fn    = SaveState.fn;
  rls->sp    = SaveState.sp;
  rls->pc    = SaveState.pc;
  rls->fp    = SaveState.fp;
  rls->bp    = SaveState.bp;
}

void discard_ctx(void) {
  ep--;
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
    system_error("out of memory");

  return out;
}

char* duplicates(RlState* rls, char* cs) {
  (void)rls; // for consistency
  char* out = strdup(cs);

  if ( out == NULL )
    system_error("out of memory");

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
          system_error("out of memory");

        rls->vm->heap_used -= diff;
      } else if ( o < n ) {
        size_t diff = n - o;

        if ( check_gc(rls, diff) )
          run_gc(rls);

        out = realloc(spc, n);

        if ( out == NULL )
          system_error("out of memory");

        memset(out+o, 0, diff);

        rls->vm->heap_used += diff;
      }
    } else {
      out = realloc(spc, n);

      if ( out == NULL )
        system_error("out of memory");

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

void next_gc_frame(GcFrame* gcf) {
  assert(gcf != NULL);

  GcFrames = gcf->next;
}

