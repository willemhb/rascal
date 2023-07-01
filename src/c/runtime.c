#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>


#include "runtime.h"
#include "lang.h"
#include "object.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NHEAP  131072
#define NSTACK 65536

value_t TheStack[NSTACK];

vm_t Vm = {
  
};

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// error handling -------------------------------------------------------------
static void print_error( const char* fname, value_t cause, const char* fmt, va_list va ) {
  fprintf(stderr, "error in %s: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  fprintf(stderr, "caused by: ");
  print(stderr, cause);
  fprintf(stderr, "\n");
}

void error( const char* fname, value_t cause, const char* fmt, ... ) {
  va_list va;
  va_start(va, fmt);
  print_error(fname, cause, fmt, va);
  va_end(va);
  panic();
}

void require( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( !test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    panic();
  }
}

void forbid( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    panic();
  }
}

// memory ---------------------------------------------------------------------
static bool overflows_heap( usize nbytes ) {
  return Vm.used + nbytes > Vm.cap;
}

void manage( void ) {
  Vm.managing = true;

  // mark roots
  mark_object(&Vm.stack);
  mark_object(&Vm.globals.vars);
  mark_object(&Vm.globals.vals);
  mark_value(Vm.symbolTable);
  mark_object(&Vm.dispatch);
  mark_object(&Vm.expressions);
  mark_object(&Vm.buffer);

  // begin tracing
  while ( Vm.grays.cnt ) {
    value_t v = vector_pop(&Vm.grays);
    trace_value(v);
  }

  // free unused objects
  object_t* curr = Vm.live, **prev = &Vm.live;

  while ( curr ) {
    if ( curr->black ) {
      unmark_object(curr);
      prev = &curr->next;
      curr = curr->next;
    } else {
      object_t* tmp = curr;
      curr = tmp->next;
      *prev = curr;
      free_object(tmp);
    }
  }

  // unmark global objects
  unmark_object(&Vm.stack);
  unmark_object(&Vm.globals.vars);
  unmark_object(&Vm.globals.vals);
  unmark_object(&Vm.dispatch);
  unmark_object(&Vm.expressions);
  unmark_object(&Vm.buffer);

  Vm.managing = false;
}
