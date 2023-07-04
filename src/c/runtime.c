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
#define NFRAME 8192

value_t TheStack[NSTACK];
value_t TheFrame[NFRAME];

vm_t Vm = {
  // main registers
  .ip=NULL,
  .sp=TheStack,
  .fp=TheFrame,
  .bp=NULL,
  .fn=NULL,

  // globals & symbol table
  .globals={
    .vars={
      .cnt=0,
      .cap=0,
      .data=NULL
    },
    .vals={
      .cnt=0,
      .cap=0,
      .data=NULL
    }
  },

  .symbolTable=NULL,
  .symbolCounter=0,

  // heap
  .used=0,
  .cap=NHEAP,
  .managing=false,
  .live=NULL,
  .grays={
    .cnt=0,
    .cap=0,
    .data=NULL
  },

  // reader
  .token=TOKEN_READY,
  .source=NULL,
  .dispatch={
    .cnt=0,
    .cap=0,
    .data=NULL
  },
  .expressions={
    .cnt=0,
    .cap=0,
    .data=NULL
  },
  .buffer={
    .cnt=0,
    .cap=0,
    .data=NULL,
    .elSize=1,
    .encoding=ASCII
  },

  // error
  .error=NO_ERROR,
  .cause=NIL,
  .message=""
};

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// stack ----------------------------------------------------------------------
value_t* push( value_t x ) {
  value_t* out = Vm.sp++;
  *out = x;
  return out;
}

value_t pop( void ) {
  value_t out = *(--Vm.sp);
  return out;
}

value_t* save( value_t x ) {
  value_t* out = Vm.fp++;
  *out = x;
  return out;
}

value_t restore( void ) {
  value_t out = *(--Vm.fp);
  return out;
}

bool in_stack( void* p ) {
  return (value_t*)p >= TheStack && (value_t*)p < (TheStack+NSTACK);
}

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

static void mark_stack( void ) {
  for ( value_t* slot=TheStack; slot < Vm.sp; slot++ )
    mark(*slot);
}

static void mark_frames( void ) {
  for ( value_t* slot=TheFrame; slot < Vm.fp; slot+=3 ) {
    mark(slot[0]);

    // might be captured, in which case it's the first slot of a heap-allocated tuple (mark head)
    value_t* bp = as_pointer(slot[1]);

    if ( !in_stack(bp) )
      mark(((void*)bp) - sizeof(tuple_t));
  }
}

static void mark_globals( void ) {
  mark_table(&Vm.globals.vars);
  mark_values(&Vm.globals.vals);
  mark_object(Vm.symbolTable);
}

static void mark_reader( void ) {
  mark_values(&Vm.expressions);
}

void manage( void ) {
  Vm.managing = true;

  // mark roots
  mark_frames();
  mark_stack();
  mark_globals();
  mark_reader();
  
  // begin tracing
  while ( Vm.grays.cnt ) {
    object_t* o = objects_pop(&Vm.grays);
    trace(o);
  }

  // free unused objects
  object_t* curr = Vm.live, **prev = &Vm.live;

  while ( curr ) {
    if ( curr->black ) {
      curr->black = false;
      curr->gray = true;
      prev = &curr->next;
      curr = curr->next;
    } else {
      object_t* tmp = curr;
      curr = tmp->next;
      *prev = curr;
      destruct_object(tmp);
    }
  }

  Vm.managing = false;
}
