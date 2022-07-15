#include <assert.h>
#include <stdarg.h>

#include "stack.h"
#include "error.h"

int push(value_t xv) {
  r_require( r_fname(),
	     Sp < N_STACK,
	     "stack overflow" );

  index_t out = Sp;
  Stack[Sp++] = xv;
  return out;
}

int pushn(arity_t n, ...) {
  r_require( Sp + n < N_STACK,
	   "stack overflow" );

  index_t out = Sp;
  Sp += n;

  va_list va;
  va_start(va, n);

  for (index_t i=out; i<Sp; i++) {
    value_t xv = va_arg(va,value_t);
    Stack[i] = xv;
  }

  va_end(va);
  return out;
}

value_t pop(void) {
  require( Sp,
	   "stack underflow" );
  return Stack[--Sp];
}

value_t popn(arity_t n) {
  require( n <= Sp,
	   "stack underflow" );
  value_t out = Stack[Sp-1];
  Sp -= n;
  return out;
}

// dump manipulation
index_t save(value_t x) {
  require( Dp < N_STACK,
	   "dump overflow" );

  index_t out = Dp;
  Dump[Dp++] = x;
  return out;
}

index_t saven(arity_t n, ...) {
  require( Dp + n < N_STACK,
	   "dump overflow" );

  index_t out = Dp;
  Dp += n;

  va_list va;
  va_start(va, n);

  for (index_t i=out; i<Dp; i++) {
    value_t xv = va_arg(va,value_t);
    Dump[i] = xv;
  }

  va_end(va);
  return out;
}

void unsave(void) {
  require( Dp,
	   "dump underflow" );
  Dp--;
}

void unsaven(arity_t n) {
  require( n <= Dp,
	   "dump underflow" );
  Dp -= n;
}

void restore(value_t *loc) {
  require( Dp,
	   "dump underflow" );

  *loc = Dump[--Dp];
}

void restoren(arity_t n, ...) {
  require( n <= Dp,
	   "dump underflow" );

  va_list va;
  va_start(va, n);

  for (size_t i=0; i<n; i--) {
    value_t *vloc = va_arg(va,value_t*);
    index_t  dloc = --Dp;
    if (vloc == NULL) continue;
    *vloc = Dump[dloc];
  }
  va_end(va);
}
