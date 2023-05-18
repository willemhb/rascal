// standard includes -----------------------------------------------------------
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>

// library includes -----------------------------------------------------------
#include "util/string.h"
#include "util/number.h"
#include "util/hashing.h"
#include "util/io.h"

// miscellaneous macros -------------------------------------------------------
#define unreachable __builtin_unreachable

// C types --------------------------------------------------------------------
typedef uword value_t;
typedef double number_t;
typedef struct object object_t;
typedef struct symbol symbol_t;
typedef struct list list_t;
typedef struct native native_t;
typedef struct closure closure_t;
typedef struct environment environment_t;

typedef enum {
  NOTYPE,
  SYMBOL,
  LIST,
  NATIVE,
  CLOSURE,
  ENVIRONMENT,
  OBJECT=CLOSURE,
  NUMBER,
  UNIT
} type_t;

typedef enum {
  // general flags
  BLACK   =0x8000,
  GRAY    =0x4000,
  NOFREE  =0x2000,
  FROZEN  =0x1000,

  // symbol flags
  INTERNED=0x0001,
  LITERAL =0x0002,
  CONSTANT=0x0004,
  SPCLFORM=0x0008,

  // function flags
  VARIADIC=0x0001,
} objfl_t;

struct object {
  object_t* next;   // presently unused
  type_t    type;
  flags     flags;
};

#define HEADER object_t obj

struct symbol {
  HEADER;
  char* name;
  value_t bind; // toplevel
  uint64 idno;
  symbol_t* left, * right;
};

struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

struct native {
  HEADER;
  symbol_t* name;
  usize     arity;
  value_t (*native)(usize n, value_t* args); // native
};

struct closure {
  HEADER;
  symbol_t*      name;
  usize          arity;
  list_t*        formals;
  list_t*        body;
  environment_t* environment;
};

struct environment {
  HEADER;
  environment_t* next;
  list_t*        names;
  list_t*        binds;
};

// globals --------------------------------------------------------------------
// tags
#define QNAN    0x7ff8000000000000ul
#define SIGN    0x8000000000000000ul

#define NILTAG  0x7ffc000000000000ul
#define OBJTAG  0xffff000000000000ul

#define TAGMASK 0xffff000000000000ul
#define VALMASK 0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define UNDEFINED (NILTAG|1)
#define UNBOUND   (NILTAG|3)

#define NHEAP   65536

symbol_t* SymbolTable = NULL;
uint64 SymbolCounter = 0;

usize HeapCap = NHEAP * sizeof(value_t), HeapUsed = 0;

jmp_buf SafePoint;

// runtime --------------------------------------------------------------------
// error handling -------------------------------------------------------------
void rlprint(FILE* ios, value_t x);

void error(value_t cause, const char* fmt, ...) {
  fprintf(stderr, "error: ");
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  fprintf(stderr, "caused by: ");
  rlprint(stderr, cause);
  fprintf(stderr, "\n");
  va_end(va);
  longjmp(SafePoint, 1);
}

#define require( test, cause, fmt, ... )             \
  do {                                               \
    if ( !(test) )                                   \
      error(cause, (fmt) __VA_OPT__(,) __VA_ARGS__); \
  } while (false)

#define forbid( test, cause, fmt, ... )                 \
  do {                                                  \
    if ( test )                                         \
      error(cause, (fmt) __VA_OPT__(,) __VA_ARGS__);    \
  } while (false)

#define argco( fn, xpr, nargs )                                         \
  do {                                                                  \
    if ( has_flag(fn, VARIADIC) )                                       \
      require((fn)->arity >= nargs,                                     \
              xpr,                                                      \
              "%s wanted at least %zu arguments, but got %zu",          \
              (fn)->name->name,                                         \
              (fn)->arity,                                              \
              nargs);                                                   \
    else                                                                \
      require((fn)->arity == nargs,                                     \
              xpr,                                                      \
              "%s wanted %zu arguments, but got %zu",                   \
              (fn)->name->name,                                         \
              (fn)->arity,                                              \
              nargs);                                                   \
  } while (false)

// memory ---------------------------------------------------------------------
static bool overflows_heap( usize nbytes ) {
  return HeapUsed + nbytes > HeapCap;
}

void manage( void ) {
    HeapCap >>= 1; // for now just grow the heap
}

void* allocate( usize nbytes ) {
  if ( nbytes == 0 )
    return NULL;

  if ( overflows_heap(nbytes) )
    manage();
  void* out = malloc(nbytes);
  assert(out != NULL);
  HeapUsed += nbytes;
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
    HeapUsed += diff;
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
    HeapUsed -= diff;
  } else {
    // do nothing, same size
  }

  return ptr;
}

void deallocate( void* ptr, usize nbytes ) {
  assert(nbytes <= HeapUsed);
  free(ptr);
  HeapUsed -= nbytes;
}

// special forms --------------------------------------------------------------
value_t Quote, Do, Def, Put, If, Lmb, Amp;

// main -----------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;
  return 0;
}
