#include <string.h>

#include "val/alist.h"
#include "val/seq.h"
#include "val/str.h"

#include "lang/compare.h"
#include "lang/print.h"

#include "vm/heap.h"

#include "util/hash.h"
#include "util/number.h"
#include "util/bits.h"

/* C types */
/* Forward declarations */
// Helpers
static bool   check_grow(size64 n, size64 c);
static bool   check_shrink(size64 n, size64 c);
static size64 calc_alist_size(size64 n);

// Interfaces
void trace_alist(State* vm, void* x);
void free_alist(State* vm, void* x);
void clone_alist(State* vm, void* x);

// External

/* Globals */
VTable AlistVt = {
  .code   =T_ALIST,
  .name   ="Alist",
  .obsize =sizeof(Alist),
  .tracefn=trace_alist,
  .freefn =free_alist,
  .clonefn=clone_alist,
};

/* Helpers */
static bool check_grow(size64 n, size64 c) {
  return n >= c;
}

static bool check_shrink(size64 n, size64 c) {
  return c == MIN_ARR ? n == 0 : n < (c >> 1);
}

static size64 calc_alist_size(size64 n) {
  size64 p = ceil2(n);

  return max(p, MIN_ARR);
}

/* Interfaces */
void trace_alist(State* vm, void* x) {
  Alist* a = x;

  mark_vals(vm, a->cnt, a->data);
}

void free_alist(State* vm, void* x) {
  (void)vm;

  Alist* a = x;

  // free array
  rl_dealloc(NULL, a->data, 0);

  // re-initialize
  init_alist(a);
}

/* API */
Alist* new_alist(void) {
  Alist* a = new_obj(&Vm, T_ALIST, 0);

  init_alist(a);

  return a;
}

void init_alist(Alist* a) {
  a->cnt  = 0;
  a->cap  = 0;
  a->data = NULL;
}

void grow_alist(Alist* a, size64 n) {
  size64 newc = calc_alist_size(n);
  size64 news = newc * sizeof(Val);
  size64 olds = a->cap * sizeof(Val);
  Val*   newd = rl_realloc(NULL, a->data, olds, news);
  a->cap      = newc;
  a->data     = newd;
}

void shrink_alist(Alist* a, size64 n) {
  if ( n == 0 )
    free_alist(&Vm, a);

  else {
    // compute, realloc, and set
    size64 newc = calc_alist_size(n);
    Val*   newd = rl_realloc(NULL, a->data, a->cap*sizeof(Val), newc*sizeof(Val));

    a->cap  = newc;
    a->data = newd;
  }
}

void resize_alist(Alist* a, size64 n) {
  if ( check_grow(n, a->cap) )
    grow_alist(a, n);

  else if ( check_shrink(n, a->cap) )
    shrink_alist(a, n);
}

// external methods
Alist* mk_alist(size64 n, Val* d) {
  Alist* a = new_alist();

  if ( d ) {
    grow_alist(a, n);
    memcpy(a->data, d, n*sizeof(Val));
  }

  return a;
}

Val alist_ref(Alist* a, size64 n) {
  assert(a->cnt < n);

  return a->data[n];
}

size64 alist_add(Alist* a, Val x) {
  if ( check_grow(a->cnt+1, a->cap) )
    grow_alist(a, a->cnt+1);

  size64 i   = a->cnt++;
  a->data[i] = x;

  return i;
}

size64 alist_wrt(Alist* a, size64 n, Val* d) {
  // otherwise buffer may point to invalid memory
  assert(n > 0);

  if ( check_grow(a->cnt+n, a->cap) )
    grow_alist(a, a->cnt+n);

  size64 o = a->cnt;

  if ( d ) {
    memcpy(a->data, d, n*sizeof(Val));
    a->cnt += n;
  }

  return o;
}

void alist_set(Alist* v, size64 n, Val x) {
  assert(n < v->cnt);
  v->data[n] = x;
}

Val alist_pop(Alist* a) {
  assert(a->cnt > 0);

  size64 o = --a->cnt;
  Val r = a->data[o];

  if ( check_shrink(o, a->cap) )
    shrink_alist(a, o);

  return r;
}

Val alist_popn(Alist* a, size64 n, bool e) {
  assert(n <= a->cnt);

  Val r   = a->data[e ? a->cnt-1 : a->cnt-n];
  a->cnt -= n;

  if ( check_shrink(a->cnt, a->cap) )
    shrink_alist(a, a->cnt);

  return r;
}

void alist_cat(Alist* x, Alist* y) {
  if ( y->cnt == 0 )
    return;

  alist_wrt(x, y->cnt, y->data);
}

/* Initialization */
