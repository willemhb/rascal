#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>


#include "runtime.h"
#include "lang.h"
#include "object.h"

#include "util/memory.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NHEAP  131072
#define NVALUES 65536
#define NFRAMES 8192

value_t Values[NVALUES];
frame_t Frames[NFRAMES];

vm_t Vm = {
  // main registers
  .frame={
    .fn =NULL,
    .ip =0,
    .bp =0,
    .env=NULL
  },

  // stack pointers
  .sp=0,
  .fp=0,

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
  }
};

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// stack ----------------------------------------------------------------------
value_t* push( value_t x ) {
  require("exec", Vm.sp < NVALUES, UNDEFINED, "stack overflow");
  value_t* out = &Values[Vm.sp++];
  *out = x;
  return out;
}

value_t pop( void ) {
  require("exec", Vm.sp > 0, UNDEFINED, "stack underflow");
  value_t out = Values[--Vm.sp];
  return out;
}

void push_frame( void ) {
  require("exec", Vm.fp < NFRAMES, UNDEFINED, "call stack overflow");
  Frames[Vm.fp++] = Vm.frame;
}

void pop_frame( void ) {
  require("exec", Vm.fp > 0, UNDEFINED, "call stack underflow");
  Vm.sp = Vm.frame.bp;
  Vm.frame = Frames[--Vm.fp];
}

// error handling -------------------------------------------------------------
static void print_error( const char* fname, value_t cause, const char* fmt, va_list va ) {
  fprintf(stderr, "error in %s: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  fprintf(stderr, "caused by: ");
  if ( cause != UNDEFINED )
    print(stderr, cause);
  fprintf(stderr, "\n");
}

void error( const char* fname, value_t cause, const char* fmt, ... ) {
  va_list va;
  va_start(va, fmt);
  print_error(fname, cause, fmt, va);
  va_end(va);
  longjmp(Vm.context, 1);
}

void require( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( !test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    longjmp(Vm.context, 1);
  }
}

void forbid( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    longjmp(Vm.context, 1);
  }
}

// memory ---------------------------------------------------------------------
static bool overflows_heap( usize nbytes ) {
  return Vm.used + nbytes > Vm.cap;
}

static void mark_stack( void ) {
  for ( int i=0; i < Vm.sp; i++ )
    mark(Values[i]);
}

static void mark_frames( void ) {
  mark(FN);
  mark(ENV);

  for ( int i=0; i < Vm.fp; i++ ) {
    mark(Frames[i].fn);
    mark(Frames[i].env);
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
      destruct(tmp);
    }
  }

  Vm.managing = false;
}

void* allocate( usize nBytes, bool fromHeap ) {
  if ( fromHeap ) {
    if ( overflows_heap(nBytes) )
      manage();

    Vm.used += nBytes;
  }

  void* out = SAFE_ALLOC(malloc, nBytes);
  memset(out, 0, nBytes);
  return out;
}

void* reallocate( void* ptr, usize oldSize, usize newSize, bool fromHeap ) {
  void* out;
  
  if ( fromHeap ) {
    if ( newSize > oldSize ) {
      usize diff = newSize - oldSize;
      
      if ( overflows_heap(diff) )
        manage();

      Vm.used += diff;
      out = SAFE_ALLOC(realloc, ptr, newSize);
      memset(out+oldSize, 0, diff);
    } else {
      usize diff = oldSize - newSize;
      out = SAFE_ALLOC(realloc, ptr, newSize);
      Vm.used -= diff;
    }
  } else {
    out = SAFE_ALLOC(realloc, ptr, newSize);

    if ( newSize > oldSize )
      memset(out+oldSize, 0, newSize - oldSize);
  }

  return out;
}

void* duplicate( void* ptr, usize nBytes, bool fromHeap ) {
  void* copy = allocate(nBytes, fromHeap);
  memcpy(copy, ptr, nBytes);
  return copy;
}

char* duplicates( char* str, bool fromHeap ) {
  return duplicate(str, strlen(str)+1, fromHeap);
}

void deallocate( void* ptr, usize nBytes, bool fromHeap ) {
  free(ptr);

  if ( fromHeap )
    Vm.used -= nBytes;
}
