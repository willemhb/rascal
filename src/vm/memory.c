#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm/vm.h"
#include "vm/error.h"
#include "vm/memory.h"
#include "val.h"
#include "util/collection.h"

// magic numbers
#define HEAPLOAD 0.75

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
