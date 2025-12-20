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
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
GcFrame* GcFrames = NULL;

VM Vm = {
  .upvs = NULL,
  .fn   = NULL,
  .pc   = NULL,
  .sp   = 0,
  .fp   = 0,
  .bp   = 0
};

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
static bool check_gc(size_t n);
static void mark_vm(void);
static void mark_globals(void);
static void mark_upvals(void);
static void mark_types(void);
static void mark_gc_frames(void);
static void mark_phase(void);
static void trace_phase(void);
static void sweep_phase(void);
static void cleanup_phase(void);

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
  restore_ctx();
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
void reset_stack(void) {
  memset(Vm.stack, 0, EXPR_STACK_SIZE * sizeof(Expr));
  Vm.sp = 0;
}

Expr* stack_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Vm.sp;

  if ( j < 0 || j > Vm.sp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Vm.stack[j];
}

Expr* push( Expr x ) {
  if ( Vm.sp == EXPR_STACK_SIZE )
    runtime_error("stack overflow");

  Vm.stack[Vm.sp] = x;

  return &Vm.stack[Vm.sp++];
}

Expr* pushn( int n ) {
  if ( Vm.sp + n >= EXPR_STACK_SIZE )
    runtime_error("stack overflow");

  Expr* base = &Vm.stack[Vm.sp]; Vm.sp += n;

  return base;
}

Expr pop( void ) {
  if ( Vm.sp == 0 )
    runtime_error("stack underflow");

  return Vm.stack[--Vm.sp];
}

Expr rpop(void) {
  if ( Vm.sp < 2 )
    runtime_error("stack underflow");

  Expr out          = Vm.stack[Vm.sp-2];
  Vm.stack[Vm.sp-2] = Vm.stack[Vm.sp-1];

  Vm.sp--;

  return out;
}

Expr popn( int n ) {
  if ( n > Vm.sp )
    runtime_error("stack underflow");

  Expr out = Vm.stack[Vm.sp-1]; Vm.sp -= n;

  return out;
}

// return an UpVal object corresponding to the given stack location
UpVal* get_upv(Expr* loc) {
  UpVal** spc = &Vm.upvs;
  UpVal* out = NULL;

  while ( out == NULL ) {
    UpVal* upv = *spc;

    if ( upv == NULL || loc < upv->loc )
      *spc = out = mk_upval(upv, loc);

    else if ( upv->loc == loc )
      out = upv;

    else
      spc = &upv->next;
  }

  return out;
}

void close_upvs(Expr* base) {
  UpVal* upv = Vm.upvs;

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
void install_fun(Fun* fun, int argc) {
  Vm.fn = fun;
  Vm.pc = fun->chunk->code->binary.vals;
  Vm.bp = Vm.sp-argc;
}

void save_frame(void) {
  if ( Vm.fp == CALL_STACK_SIZE )
    runtime_error("runtime:save_frame", "stack overflow");

  CallState *frame = &Vm.frames[Vm.fp++];
  frame->frame_size = Vm.sp-Vm.bp;
  frame->cntl_off = -1;
  frame->flags = -1;
  frame->savepc = Vm.pc;
}

void restore_frame(void) {
  assert(Vm.fp > 0);
  CallState* frame = &Vm.frames[--Vm.fp];

  
  Vm.pc       = as_ptr(frame[0]);
  Vm.fn       = as_fun(frame[1]);
  Vm.bp       = as_fix(frame[2]);
  Vm.fp       = as_fix(frame[3]);
}

void reset_vm(void) {
  Vm.upvs = NULL;
  Vm.pc   = NULL;
  Vm.fn   = NULL;
  Vm.fp   =  0;
  Vm.bp   =  0;
  Vm.sp   =  0;
}

// garbage collector
static bool check_gc(size_t n) {
  return HeapUsed + n >= HeapCap;
}

static bool check_heap_grow(void) {
  return HeapCap * HEAPLOAD < HeapUsed;
}

static void mark_vm(void) {
  mark_obj(Vm.fn);

  for ( int i=0; i < Vm.sp; i++ )
    mark_exp(Vm.stack[i]);
}

static void mark_globals(void) {
  extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr;

  mark_obj(&Globals);
  mark_obj(QuoteStr);
  mark_obj(DefStr);
  mark_obj(PutStr);
  mark_obj(IfStr);
  mark_obj(DoStr);
  mark_obj(FnStr);
}

static void mark_upvals(void) {
  UpVal* upv = Vm.upvs;

  while ( upv != NULL ) {
    mark_obj(upv);
    upv = upv->next;
  }
}

static void mark_types(void) {
  for ( int i=0; i < NUM_TYPES; i++ )
    mark_obj(Types[i].repr);
}

static void mark_gc_frames(void) {
  GcFrame* frame = GcFrames;

  while ( frame != NULL ) {
    for ( int i=0; i < frame->count; i++ )
      mark_exp(frame->exprs[i]);

    frame = frame->next;
  }
}

static void mark_phase(void) {
  mark_vm();
  mark_globals();
  mark_upvals();
  mark_types();
  mark_gc_frames();
}

static void trace_phase(void) {
  while ( GrayStack.count > 0 ) {
    Obj* obj          = objs_pop(&GrayStack);
    ExpTypeInfo* info = &Types[obj->type];
    obj->gray         = false;
    
    info->trace_fn(obj);
  }
}

static void sweep_phase(void) {
  Obj** spc = &Heap;

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

static void cleanup_phase(void) {
  if ( check_heap_grow() )
    HeapCap <<= 1;
}

void add_to_heap(void* ptr) {
  Obj* obj  = ptr;
  obj->heap = Heap;
  Heap      = obj;
}

void gc_save(void* ob) {
  objs_push(&GrayStack, ob);
}

void run_gc(void) {
#ifdef RASCAL_DEBUG
  printf("\n\nINFO: entering gc.\n\n");
  heap_report();
#endif

  mark_phase();
  trace_phase();
  sweep_phase();
  cleanup_phase();

#ifdef RASCAL_DEBUG
  printf("\n\nINFO: exiting gc.\n\n");
  heap_report();
#endif

}

void heap_report(void) {
  printf("\n\n==== heap report ====\n\%-16s %20zu\n%-16s %20zu\n\n",
         "allocated",
         HeapUsed,
         "used",
         HeapCap);
}

void save_ctx(void) {
  // The Toplevel jmp_buf will probably be preserved and used for something 
  assert(ep < MAX_SAVESTATES);

  VmCtx* ctx = &SaveStates[ep++];

  ctx->fn = Vm.fn;
  ctx->pc = Vm.pc;
  ctx->sp = Vm.sp;
  ctx->fp = Vm.fp;
  ctx->bp = Vm.bp;
}

void restore_ctx(void) {
  // close any upvalues whose frames are being abandoned
  close_upvs(&Vm.stack[SaveState.sp]);

  // discard invalidated GC frames (important because these now point to invalid memory)
  GcFrames = SaveState.gcf;

  // restore main registers
  Vm.fn    = SaveState.fn;
  Vm.sp    = SaveState.sp;
  Vm.pc    = SaveState.pc;
  Vm.fp    = SaveState.fp;
  Vm.bp    = SaveState.bp;
}

void discard_ctx(void) {
  ep--;
}

void* allocate(bool h, size_t n) {
  void* out;

  if ( h ) {
    if ( check_gc(n) )
      run_gc();

    out = calloc(n, 1);
    HeapUsed += n;
  } else
    out = calloc(n, 1);

  if ( out == NULL )
    system_error("out of memory");

  return out;
}

char* duplicates(char* cs) {
  char* out = strdup(cs);

  if ( out == NULL )
    system_error("out of memory");

  return out;
}

void* duplicate(bool h, size_t n, void* ptr) {
  void* spc = allocate(h, n);

  memcpy(spc, ptr, n);

  return spc;
}

void* reallocate(bool h, size_t n, size_t o, void* spc) {
  void* out;

  if ( n == 0 ) {
    out = NULL;

    release(spc, o * h);
  } else if ( o == 0 ) {
    out = allocate(h, n);
  } else {
    if ( h ) {
      if ( o > n ) {
        size_t diff = o - n;
        out         = realloc(spc, n);

        if ( out == NULL )
          system_error("out of memory");

        HeapUsed   -= diff;
      } else if ( o < n ) {
        size_t diff = n - o;

        if ( check_gc(diff) )
          run_gc();

        out = realloc(spc, n);

        if ( out == NULL )
          system_error("out of memory");

        memset(out+o, 0, diff);

        HeapUsed += diff;
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

void release(void* spc, size_t n) {
  free(spc);
  HeapUsed -= n;
}

void next_gc_frame(GcFrame* gcf) {
  assert(gcf != NULL);

  GcFrames = gcf->next;
}

