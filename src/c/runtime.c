#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>


#include "runtime.h"
#include "print.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NHEAP   65536

struct Heap Heap = {
  .cap  =NHEAP,
  .used =0,
  .live =NULL,
  .grays={
    .obj={
      .next =NULL,
      .type =VECTOR,
      .flags=NOFREE|BLACK,
    },
    .cnt =0,
    .cap =0,
    .data=NULL
  }
};

// error state ----------------------------------------------------------------
struct Error Error = {
  .cause=NIL
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

void panic( value_t cause ) {
  Error.cause = cause;
  longjmp(Error.safety, 1);
}

void error( const char* fname, value_t cause, const char* fmt, ... ) {
  va_list va;
  va_start(va, fmt);
  print_error(fname, cause, fmt, va);
  va_end(va);
  panic(cause);
}

void require( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( !test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    panic(cause);
  }
}

void forbid( const char* fname, bool test, value_t cause, const char* fmt, ... ) {
  if ( test ) {
    va_list va;
    va_start(va, fmt);
    print_error(fname, cause, fmt, va);
    va_end(va);
    panic(cause);
  }
}

// memory ---------------------------------------------------------------------
static bool overflows_heap( usize nbytes ) {
  return Heap.used + nbytes > Heap.cap;
}

void manage( void ) {
    Heap.cap >>= 1; // for now just grow the heap
}

void* allocate( usize nbytes ) {
  if ( nbytes == 0 )
    return NULL;

  if ( overflows_heap(nbytes) )
    manage();
  void* out = malloc(nbytes);
  assert(out != NULL);
  Heap.used += nbytes;
  memset(out, 0, nbytes);
  return out;
}

void* reallocate( void* ptr, usize oldSize, usize newSize ) {
  usize diff;
  
  if ( ptr == NULL ) {
    assert(oldSize == 0);
    ptr = allocate(newSize);
  } else if ( newSize > oldSize ) {
    diff = newSize - oldSize;

    if ( overflows_heap(diff) )
      manage();

    ptr = realloc(ptr, newSize);
    assert(ptr != NULL);
    Heap.used += diff;
    memset(ptr+oldSize, 0, diff);
  } else if ( newSize < oldSize ) {
    diff = oldSize - newSize;

    if ( newSize == 0 ) {
      free(ptr);
      ptr = NULL;
    } else {
      ptr = realloc(ptr, newSize);
      assert(ptr != NULL);
    }
    Heap.used -= diff;
  } else {
    // do nothing, same size
  }

  return ptr;
}

void deallocate( void* ptr, usize nbytes ) {
  assert(nbytes <= Heap.used);
  if ( ptr == NULL ) {
    assert(nbytes == 0);
  } else {
    free(ptr);
    Heap.used -= nbytes;
  }
}

void* duplicate( void* ptr, usize nbytes ) {
  void* out = allocate(nbytes);
  memcpy(out, ptr, nbytes);
  return out;
}

char* duplicate_str(char* str) {
  usize n = strlen(str);
  char* out = allocate(n+1);
  memcpy(out, str, n);
  return out;
}
