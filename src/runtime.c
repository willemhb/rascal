#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"
#include "collection.h"

// magic numbers
#define HEAPLOAD 0.75

// Global declarations
char* ErrorNames[] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [EVAL_ERROR]    = "eval",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

char Token[BUFFER_SIZE];
size_t TOff = 0;
Status VmStatus = OKAY;
jmp_buf Toplevel;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
GcFrame* GcFrames = NULL;

VM Vm = {
  .fn = NULL,
  .pc = NULL,
  .sp = 0
};

Env Globals = {
  .type    = EXP_ENV,
  .black   = false,
  .gray    = true,
  .nosweep = true,

  .map = {
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

Stack GrayStack = {
  .vals      = NULL,
  .count     = 0,
  .max_count = 0
};

// internal helpers
static bool check_gc(size_t n);
static void mark_vm(void);
static void mark_globals(void);
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

  longjmp(Toplevel, 1);
}

void recover(void) {
  if ( VmStatus ) {
    VmStatus = OKAY;
    GcFrames = NULL;
    reset_token();
    reset_stack();
    fseek(stdin, SEEK_SET, SEEK_END); // clear out invalid characters
  }
}

void rascal_error(Status etype, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s error: ", ErrorNames[etype]);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
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

// stack API
void reset_stack(void) {
  memset(Vm.stack, 0, STACK_SIZE * sizeof(Expr));
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
  if ( Vm.sp == STACK_SIZE )
    runtime_error("stack overflow");

  Vm.stack[Vm.sp] = x;

  return &Vm.stack[Vm.sp++];
}

Expr* pushn( int n ) {
  if ( Vm.sp + n >= STACK_SIZE )
    runtime_error("stack overflow");

  Expr* base = &Vm.stack[Vm.sp]; Vm.sp += n;

  return base;
}

Expr pop( void ) {
  if ( Vm.sp == 0 )
    runtime_error("stack underflow");

  return Vm.stack[--Vm.sp];
}

Expr popn( int n ) {
  if ( n > Vm.sp )
    runtime_error("stack underflow");

  Expr out = Vm.stack[Vm.sp-1]; Vm.sp -= n;

  return out;
}

void install_code(Fun* fn, int bp) {
  Vm.fn = fn;
  Vm.pc = (instr_t*)fn->chunk->code->binary.vals;

  if ( bp > -1 )
    Vm.bp = bp;
}

void save_frame(void) {
  if ( Vm.fn != NULL ) {
    Expr* frame = pushn(4);
    frame[0]    = tag_ptr(Vm.pc);
    frame[1]    = tag_obj(Vm.fn);
    frame[2]    = tag_fix(Vm.fp);
    frame[3]    = tag_fix(Vm.bp);
    Vm.fp       = Vm.sp;
  }
}

void restore_frame(void) {
  assert(Vm.fp > 4);

  Expr* frame = stack_ref(Vm.fp-4);
  Vm.pc       = as_ptr(frame[0]);
  Vm.fn       = as_obj(frame[1]);
  Vm.fp       = as_fix(frame[2]);
  Vm.bp       = as_fix(frame[3]);
}

void reset_vm(void) {
  Vm.pc  = NULL;
  Vm.fn  = NULL;
  Vm.sp  = 0;
  Vm.fp  = 0;
  Vm.bp  = 0;
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
  extern Str* QuoteStr, * SetStr, * IfStr;

  mark_obj(&Globals);
  mark_obj(QuoteStr);
  mark_obj(SetStr);
  mark_obj(IfStr);
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
  mark_types();
  mark_gc_frames();
}

static void trace_phase(void) {

  while ( GrayStack.count > 0 ) {
    Obj* obj          = stack_pop(&GrayStack);
    ExpTypeInfo* info = &Types[obj->type];
    obj->gray         = false;
    
    info->trace_fn(obj);
  }
}

static void sweep_phase(void) {
  Obj** spc = &Heap;

  while ( *spc != NULL ) {
    Obj* obj = *spc;

    if ( obj->black ) {         // prserve
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

void gc_save(void* ob) {
  stack_push(&GrayStack, ob);
}

void run_gc(void) {
#ifdef RASCAL_DEBUG
  printf("\n\nINFO: entering gc.\nallocated: %zu\nused: %zu\n\n",
         HeapUsed,
         HeapCap);
#endif

  mark_phase();
  trace_phase();
  sweep_phase();
  cleanup_phase();

#ifdef RASCAL_DEBUG
  printf("\n\nINFO: exiting gc.\nallocated: %zu\nused: %zu\n\n",
         HeapUsed,
         HeapCap);
#endif

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
