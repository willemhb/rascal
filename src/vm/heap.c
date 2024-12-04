#include <string.h>
#include <stdlib.h>

#include "vm/heap.h"

#include "val/array.h"

#include "util/memory.h"

/* Globals. */
Alist Grays = {
  .tag     = T_ALIST,
  .notrace = true,
  .nosweep = true,
  .gray    = true,

  .cnt     = 0,
  .cap     = 0,
  .data    = NULL
};

static double HeapLf = 0.625;

/* Internal APIs */
static bool check_gc(State* vm, size_t n) {
  return vm->alloc + n > vm->limit;
}

static bool check_resize(State* vm) {
  return (double)vm->alloc / HeapLf >= (double)vm->limit;
}

static void grow_heap(State* vm) {
  if ( vm->limit < MAX_POW2 )
    vm->limit = MAX_ARITY;

  else
    vm->limit <<= 1;
}

static void manage_heap(State* vm, size_t s, size_t a) {
  if ( vm ) {
    if ( a > s ) {
      size_t d = a - s;

      if ( check_gc(vm, d) )
        rl_gc(vm, d);

      vm->alloc += d;
    } else if ( a < s ) {
      size_t d = s - a;

      vm->alloc -= d;
    }
  }
}

static void sweep(State* vm) {
  Obj** heap = &vm->heap;

  while ( *heap ) {
    Obj* gcobj = *heap;

    if ( gcobj->black ) { // unmark and continue
      gcobj->black = false;
      gcobj->gray  = true;
      heap       = &gcobj->heap;
    } else {
      *heap      = gcobj->heap;

      if ( !gcobj->nofree )
        free_obj(vm, gcobj);

      if ( !gcobj->nosweep )
        sweep_obj(vm, gcobj);
    }
  }
}

/* External APIs */
void* rl_alloc(State* vm, size_t n) {
  manage_heap(vm, 0, n);
  return s_malloc(n, 0);
}

void* rl_realloc(State* vm, void* s, size_t o, size_t n) {
  manage_heap(vm, o, n);
  return s_realloc(s, o, n, 0);
}

void* rl_dup(State* vm, void* s, size_t n) {
  manage_heap(vm, 0, n);
  return s_mdup(s, n);
}

void rl_dealloc(State* vm, void* s, size_t n) {
  manage_heap(vm, n, 0);
  s_free(s);
}

void rl_gc(State* vm, size_t n);

void push_gray(State* vm, void* o) {
  alist_add(vm->grays, tag_obj(o));
}

void* pop_gray(State* vm) {
  Val o = alist_pop(vm->grays);

  return as_obj(o);
}

// utilities
void mark_vals(State* vm, size64 n, Val* vs) {
  for ( size64 i=0; i<n; i++ )
    mark(vm, vs[i]);
}

void mark_objs(State* vm, size64 n, void* os) {
  Obj** objs = os;

  for ( size64 i=0; i<n; i++ )
    if ( objs[i] )
      mark(vm, objs[i]);
}
