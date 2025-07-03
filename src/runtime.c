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
char* ErrorNames[NUM_ERRORS] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [EVAL_ERROR]    = "eval",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

Sym* ErrorTypes[NUM_ERRORS] = {};

VmCtx SaveStates[MAX_SAVESTATES];
int ep = 0;

char Token[BUFFER_SIZE];
size_t TOff = 0;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
GcFrame* GcFrames = NULL;

Expr Vals[N_VALS];
Expr Frames[N_FRAMES];

VM Vm = {
  .upvs   = NULL,
  .fn     = NULL,
  .pc     = NULL,
  .sp     = 0,
  .fp     = 0,
  .bp     = 0,
  .frames = Frames,
  .vals   = Vals
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


// mostly whitespace and control characters

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

  longjmp(SaveState.Cstate, etype);
}

void recover(funcptr_t cleanup) {
  restore_ctx();

  if ( cleanup )
    cleanup();
}

void rascal_error(Status etype, char* fmt, ...) {
  if ( etype > USER_ERROR ) { // user error messages handled in rascal code
    va_list va;
    va_start(va, fmt);
    pprintf(&Errs, "%s error: ", ErrorNames[etype]);
    pvprintf(&Errs, fmt, va);
    pprintf(&Errs, ".\n");
    va_end(va);
  }

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

// vals API -------------------------------------------------------------------
void reset_vals(void) {
  memset(Vals, 0, N_VALS * sizeof(Expr));
  Vm.sp = 0;
  Vm.bp = 0;
}

Expr* vals_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Vm.sp;

  if ( j < 0 || j > Vm.sp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Vm.vals[j];
}

void setvp(int n) {
  assert(n > 0 && n <= N_VALS);
  Vm.sp = n;
}

Expr* vpush( Expr x ) {
  if ( Vm.sp == N_VALS )
    runtime_error("stack overflow");

  Vm.vals[Vm.sp] = x;

  return &Vm.vals[Vm.sp++];
}

Expr* vpushn( int n ) {
  if ( Vm.sp + n >= N_VALS )
    runtime_error("stack overflow");

  Expr* base = &Vm.vals[Vm.sp]; Vm.sp += n;

  return base;
}

Expr vpop( void ) {
  if ( Vm.sp == 0 )
    runtime_error("stack underflow");

  return Vm.vals[--Vm.sp];
}

Expr vrpop(void) {
  if ( Vm.sp < 2 )
    runtime_error("stack underflow");

  Expr out = Vm.vals[Vm.sp-2];
  Vm.vals[Vm.sp-2] = Vm.vals[Vm.sp-1];

  Vm.sp--;

  return out;
}

Expr vpopn( int n ) {
  if ( n > Vm.sp )
    runtime_error("stack underflow");

  Expr out = Vm.vals[Vm.sp-1]; Vm.sp -= n;

  return out;
}

void reset_frames(void) {
  memset(Frames, 0, N_FRAMES * sizeof(Expr));
  Vm.fp = 0;
  Vm.ap = 0;
  Vm.cp = 0;
}

Expr* frames_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Vm.fp;

  if ( j < 0 || j > Vm.fp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Vm.frames[j];
}

void setfp(int n) {
  assert(n > 0 && n <= N_FRAMES);
  Vm.fp = n;
}

Expr* fpush(Expr x) {
  if ( Vm.fp == N_FRAMES )
    runtime_error("stack overflow");

  Vm.frames[Vm.fp] = x;

  return &Vm.frames[Vm.fp++];
}

Expr* fpushn(int n) {
  if ( Vm.fp + n >= N_FRAMES )
    runtime_error("stack overflow");

  Expr* base = &Vm.frames[Vm.fp]; Vm.fp += n;

  return base;
}

Expr fpop(void) {
  if ( Vm.fp == 0 )
    runtime_error("stack underflow");

  return Vm.frames[--Vm.fp];
}

Expr frpop(void) {
  if ( Vm.fp < 2 )
    runtime_error("stack underflow");

  Expr out = Vm.frames[Vm.fp-2];
  Vm.frames[Vm.fp-2] = Vm.frames[Vm.fp-1];

  Vm.fp--;

  return out;
}

Expr fpopn(int n) {
  if ( n > Vm.fp )
    runtime_error("stack underflow");

  Expr out = Vm.frames[Vm.fp-1]; Vm.fp -= n;

  return out;
}

// frame manipulation ---------------------------------------------------------
void install_fun(Fun* fun, int bp) {
  Vm.fn = fun;
  Vm.pc = user_fn_instr(fun);
  Vm.bp = bp;
}

void save_frame(void) {
  // get frame
  Expr* frame = fpushn(FRAME_SIZE);
  
  // save caller state in frame
  frame[0] = tag_obj(Vm.fn);
  frame[1] = tag_ptr(Vm.pc);
  frame[2] = tag_fix(Vm.bp);
}

void restore_frame(void) {
  // restore caller state
  // becomes new sp
  int oldbp   = Vm.bp;

  // get frame to restore
  Expr* frame = frames_ref(Vm.fp-3);

  // get saved values/offsets
  Fun* caller = as_fun(frame[0]);
  instr_t* pc = as_ptr(frame[1]);
  int rtnbp   = as_fix(frame[2]);

  // restore values
  Vm.fn = caller;
  Vm.pc = pc;
  Vm.bp = rtnbp;

  // reset vp/fp
  Vm.sp = oldbp;
}

// other vm stuff -------------------------------------------------------------
void reset_vm(void) {
  Vm.upvs = NULL;
  Vm.pc   = NULL;
  Vm.fn   = NULL;

  reset_vals();
  reset_frames();
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
    mark_exp(Vm.vals[i]);

  for ( int i=0; i < Vm.fp; i++ )
    mark_exp(Vm.frames[i]);
}

static void mark_globals(void) {
  extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr,
    * CatchStr, * ThrowStr;

  mark_obj(&Globals);
  mark_obj(QuoteStr);
  mark_obj(DefStr);
  mark_obj(PutStr);
  mark_obj(IfStr);
  mark_obj(DoStr);
  mark_obj(FnStr);
  mark_obj(CatchStr);
  mark_obj(ThrowStr);
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

static void mark_etypes(void) {
  for ( int i=0; i < NUM_ERRORS; i++ )
    mark_obj(ErrorTypes[i]);
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
  mark_etypes();
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
  assert(ep < MAX_SAVESTATES);

  VmCtx* ctx = &SaveStates[ep++];
  ctx->gcf   = GcFrames;
  ctx->fn    = Vm.fn;
  ctx->pc    = Vm.pc;
  ctx->sp    = Vm.sp;
  ctx->fp    = Vm.fp;
}

void restore_ctx(void) {
  // close any upvalues that are about to lose their place on the stack
  close_upvs(vals_ref(SaveState.sp));
  // discard invalidated GC frames (important because these now point to invalid memory)
  GcFrames = SaveState.gcf;

  // restore execution state
  Vm.fn = SaveState.fn;
  Vm.pc = SaveState.pc;
  Vm.sp = SaveState.sp;
  Vm.fp = SaveState.fp;
  Vm.bp = SaveState.bp;
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

