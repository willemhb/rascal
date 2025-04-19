#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"

// Global declarations
char* ErrorNames[] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

char Token[BUFFER_SIZE];
size_t TOff = 0;
Status VmStatus = OKAY;
jmp_buf Toplevel;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
Expr Stack[STACK_SIZE];
int Sp = 0;

// internal helpers
static bool check_gc(size_t n) {
  return HeapUsed + n >= HeapCap;
}

// error helpers
void panic(Status etype) {
  if ( etype == SYSTEM_ERROR )
    exit(1);

  longjmp(Toplevel, 1);
}

void recover(void) {
  if ( VmStatus ) {
    VmStatus = OKAY;
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
  memset(Stack, 0, STACK_SIZE * sizeof(Expr));
  Sp = 0;
}

Expr* stack_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Sp;

  if ( j < 0 || j > Sp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Stack[j];
}

Expr* push( Expr x ) {
  if ( Sp == STACK_SIZE )
    runtime_error("stack overflow");

  Stack[Sp] = x;

  return &Stack[Sp++];
}

Expr* pushn( int n ) {
  if ( Sp + n >= STACK_SIZE )
    runtime_error("stack overflow");

  Expr* base = &Stack[Sp]; Sp += n;

  return base;
}

Expr pop( void ) {
  if ( Sp == 0 )
    runtime_error("stack underflow");

  return Stack[--Sp];
}

Expr popn( int n ) {
  if ( n > Sp )
    runtime_error("stack underflow");

  Expr out = Stack[Sp-1]; Sp -= n;

  return out;
}

void run_gc(void) {
  HeapCap <<= 1;
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
