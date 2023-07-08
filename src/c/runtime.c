#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>


#include "runtime.h"
#include "lang.h"
#include "object.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NHEAP  131072
#define NVALUES 65536
#define NFRAMES 8192

value_t Values[NVALUES];
frame_t Frames[NFRAMES];

vm_t Vm = {
  // main registers
  .fn=NULL,
  .ip=NULL,
  .bp=NULL,
  .sp=Values,
  .fp=Frames,

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

frame_t* push_frame( void ) {
  return Vm.fp++;
}

void pop_frame( void ) {
  SP = BP;
  FP--;
}

tuple_t* capture_frame( frame_t* f ) {
  if ( f->env == NULL ) {
    tuple_t* parent = f == Frames ? &EmptyTuple : capture_frame(f-1);
    f->env = mk_tuple(f->fn->envt->arity+1, f->bp);
    f->env->slots[0] = object(parent);
  }
  return f->env;
}

bool in_stack( void* p ) {
  return (value_t*)p >= Values && (value_t*)p < (Values+NVALUES);
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
  for ( value_t* slot=Values; slot < Vm.sp; slot++ )
    mark(*slot);
}

static void mark_frames( void ) {
  for ( frame_t* slot=Frames; slot < Vm.fp; slot++ ) {
    mark(slot->fn);

    // might be captured
    if ( !in_stack(slot->bp) )
      mark(slot->env);
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
