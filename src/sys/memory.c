/**
 *
 * Data structures and definitions for Rascal memory management.
 * 
 **/

// headers --------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys/memory.h"
#include "sys/error.h"

#include "data/base.h"
#include "data/upv.h"

#include "lang/base.h"

// macros ---------------------------------------------------------------------
#define HEAPLOAD 0.75

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
// stores live but untraced objects during GC
Objs GrayStack = {
  .vals      = NULL,
  .count     = 0,
  .max_count = 0
};

// function prototypes --------------------------------------------------------
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

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
static bool check_gc(size_t n) {
  return HeapUsed + n >= HeapCap;
}

static bool check_heap_grow(void) {
  return HeapCap * HEAPLOAD < HeapUsed;
}

static void mark_vm(void) {
  mark_obj(Vm.fn);
  trace_exp_array(Vm.sp, Vm.stack);
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
    mark_obj(Types[i].rl_name);
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
    Obj* obj    = objs_pop(&GrayStack);
    ObjAPI* api = obj_api(obj);
    obj->gray   = false;
    
    api->trace_fn(obj);
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

// external -------------------------------------------------------------------
// high level heap API --------------------------------------------------------
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

// low level heap helpers -----------------------------------------------------
void add_to_heap(void* ptr) {
  Obj* obj  = ptr;
  obj->heap = Heap;
  Heap      = obj;
}

void gc_save(void* ob) {
  objs_push(&GrayStack, ob);
}


void next_gc_frame(GcFrame* gcf) {
  assert(gcf != NULL);

  GcFrames = gcf->next;
}

void trace_exp_array(size_t n, Expr* xs) {
  for ( size_t i=0; i < n; i++ )
    mark_exp(xs[i]);
}

void trace_obj_array(size_t n, void* os) {
  Obj** objs = os;

  for ( size_t i=0; i < n; i++ )
    mark_obj(objs[i]);
}

void heap_report(void) {
  printf("\n\n==== heap report ====\n\%-16s %20zu\n%-16s %20zu\n\n",
         "allocated",
         HeapUsed,
         "used",
         HeapCap);
}

// initialization -------------------------------------------------------------
void toplevel_init_sys_memory(void) {}
