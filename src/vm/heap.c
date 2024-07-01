#include <string.h>
#include <stdlib.h>

#include "vm/heap.h"

#include "util/memory.h"

/* Globals. */
Alist Grays = {
  .type  = &AlistType,
  .trace = false,
  .sweep = false,
  .free  = true,
  .gray  = false,
  
  .algo   = RESIZE_STACK,
  .shrink = false,

  .data    = NULL,
  ._static = NULL,
  .cnt     = 0,
  .maxc    = 0,
  .maxs    = 0,
};

static double HeapLf = 0.625;

/* Internal APIs */
static bool check_gc(RlProc* p, size_t n) {
  return p->heaps + n > p->heapm;
}

static bool check_resize(RlProc* p) {
  return (double)p->heaps / HeapLf >= (double)p->heapm;
}

static void grow_heap(RlProc* p) {
  if ( p->heapm < MAX_POW2 )
    p->heapm = MAX_ARITY;

  else
    p->heapm <<= 1;
}

static void manage_heap(RlProc* p, size_t s, size_t a) {
  if ( p ) {
    if ( a > s ) {
      size_t d = a - s;

      if ( check_gc(p, d) )
        collect_garbage(p, d);

      p->heaps += d;
    } else if ( a < s ) {
      size_t d = s - a;

      p->heaps -= d;
    }
  }
}

static void sweep(RlProc* p) {
  Obj** gcobjs = &p->gcobjs;

  while ( *gcobjs ) {
    Obj* gcobj = *gcobjs;

    if ( gcobj->black ) { // unmark and continue
      gcobj->black = false;
      gcobj->gray  = true;
      gcobjs       = &gcobj->next;
    } else {
      *gcobjs      = gcobj->next;

      if ( gcobj->free )
        free_obj(gcobj);

      if ( gcobj->sweep )
        p->heaps -= sweep_obj(gcobj);
    }
  }
}

/* External APIs */
void* allocate(RlProc* p, size_t n) {
  manage_heap(p, 0, n);
  return rl_malloc(n, 0);
}

void* reallocate(RlProc* p, void* s, size_t o, size_t n) {
  manage_heap(p, o, n);
  return rl_realloc(s, o, n, 0);
}

void* duplicate(RlProc* p, void* s, size_t n) {
  manage_heap(p, 0, n);
  return rl_mdup(s, n);
}

void deallocate(RlProc* p, void* s, size_t n) {
  manage_heap(p, n, 0);
  rl_free(s);
}

void collect_garbage(RlProc* p, size_t n) {
  if ( !p->inited ) {
    grow_heap(p);
    if ( check_gc(p, n) ) // couldn't free enough memory
      rl_fatal_err(RUNTIME_ERROR, "collect-garbage", "couldn't free enough memory to allocate %zu bytes", n);

  } else {
    mark_rlp(p);

    while ( p->grays->cnt > 0 ) {
      Obj* o = pop_gray(p);
      trace(o);
    }

    sweep(p);

    if ( check_resize(p) )
      grow_heap(p);
  }

  if ( check_gc(p, n) ) // couldn't free enough memory
    rl_fatal_err(RUNTIME_ERROR, "collect-garbage", "couldn't free enough memory to allocate %zu bytes", n);
}

void push_gray(RlProc* p, void* o) {
  alist_push(p->grays, o);
}

Obj* pop_gray(RlProc* p) {
  return alist_pop(p->grays);
}
