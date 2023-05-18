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
  OBJECT=ENVIRONMENT,
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

  // environment flags
  TOPLEVEL=0x0001
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

typedef struct function {
  HEADER;
  symbol_t* name;
  usize     arity;
} function_t;

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

list_t EmptyList = {
  .obj={
    .next =NULL,
    .type =LIST,
    .flags=GRAY|NOFREE
  },
  .head =NIL,
  .tail =&EmptyList,
  .arity=0
};

environment_t EmptyEnvironment = {
  .obj={
    .next =NULL,
    .type =ENVIRONMENT,
    .flags=GRAY|NOFREE|TOPLEVEL
  },

  .names=&EmptyList,
  .binds=&EmptyList,
  .next =&EmptyEnvironment
};

// runtime --------------------------------------------------------------------
// error handling -------------------------------------------------------------
void print(FILE* ios, value_t x);

void error(value_t cause, const char* fmt, ...) {
  fprintf(stderr, "error: ");
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  fprintf(stderr, "caused by: ");
  print(stderr, cause);
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

// value APIs -----------------------------------------------------------------
object_t* as_object( value_t x );
usize size_of_type( type_t t );

type_t type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case OBJTAG: return as_object(x)->type;
    case NILTAG: return UNIT;
    default:     return NUMBER;
  }
}

#define size_of(x) _Generic((x), value_t: size_of_value, type_t: size_of_type)(x)

usize size_of_value( value_t x ) {
  return size_of_type( type_of(x) );
}

usize size_of_type( type_t t ) {
  switch ( t ) {
    case SYMBOL:      return sizeof(symbol_t);
    case LIST:        return sizeof(list_t);
    case NATIVE:      return sizeof(native_t);
    case CLOSURE:     return sizeof(closure_t);
    case ENVIRONMENT: return sizeof(environment_t);
    case NUMBER:      return sizeof(number_t);
    default:          return 0;
  }
}

// object APIs ----------------------------------------------------------------
// object ---------------------------------------------------------------------
value_t object( void* p ) {
  return ((value_t)p) | OBJTAG;
}

void* make_object( type_t type, flags fl ) {
  object_t* out = allocate(size_of(type));

  out->next     = NULL;
  out->type     = type;
  out->flags    = fl|GRAY;

  return out;
}

// symbol ---------------------------------------------------------------------
symbol_t* as_symbol( value_t x ) {
  return (symbol_t*)(x & VALMASK);
}

bool is_symbol( value_t x ) {
  return type_of(x) == SYMBOL;
}

symbol_t* make_symbol( char* name, flags fl ) {
  assert(name);
  symbol_t* sym = make_object(SYMBOL, fl|(*name==':') * LITERAL);
  sym->idno     = ++SymbolCounter;
  sym->left     = NULL;
  sym->right    = NULL;
  sym->name     = duplicate_str(name);
  sym->bind     = UNDEFINED;

  return sym;
}

static symbol_t** locate_symbol(char* name, symbol_t** buf) {
  while (*buf) {
    int o = strcmp(name, (*buf)->name);

    if ( o < 0 )
      buf = &(*buf)->left;

    else if ( o > 0 )
      buf = &(*buf)->right;

    else
      break;
  }

  return buf;
}

static symbol_t* intern_symbol( char* name, flags fl ) {
  symbol_t** loc = locate_symbol(name, &SymbolTable);

  if (*loc == NULL)
    *loc = make_symbol(name, INTERNED|fl);

  return *loc;
}

symbol_t* symbol( char* name, bool interned ) {
  if ( interned )
    return intern_symbol(name, 0);

  return make_symbol(name, 0);
}

// list -----------------------------------------------------------------------
list_t* as_list( value_t x ) {
  return (list_t*)(x & VALMASK);
}

bool is_list( value_t x ) {
  return type_of(x) == LIST;
}

// environment ----------------------------------------------------------------
static value_t lookup( value_t name, environment_t* envt ) {
  value_t out = UNDEFINED;

  while ( envt && out == UNDEFINED ) {
    for ( list_t* names=envt->names, * binds=envt->binds; names->arity && out == UNDEFINED; names=names->tail, binds=binds->tail ) {
      if ( name == names->head )
        out = binds->head;
    }

    envt = envt->next;
  }

  if ( out == UNDEFINED )
    out = as_symbol(name)->bind;

  return out;
}

// lang -----------------------------------------------------------------------
// read -----------------------------------------------------------------------
// print ----------------------------------------------------------------------
// eval -----------------------------------------------------------------------
// special forms --------------------------------------------------------------
value_t Quote, Do, Def, Put, If, Lmb, Amp;

static bool is_literal( value_t x );
static bool is_function( value_t x );
static bool is_special_form( value_t x );
static value_t eval_sexpr( list_t* form, environment_t* envt );

value_t eval( value_t x, environment_t* envt ) {
  value_t v;

  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) )
    v = lookup(x, envt);

  else
    v = eval_sexpr(as_list(x), envt);

  require(v != UNDEFINED, x, "unbound symbol");
  return v;
}

static value_t eval_sexpr( list_t* form, environment_t* envt ) {
  value_t v=NIL, head = form->head;
  list_t* args = form->tail;

  if ( is_special_form(head) ) {
    
  } else {
    
  }
  return v;
}

// initialization -------------------------------------------------------------
void init_rascal(void) {
  // special forms & other syntactic markers ----------------------------------
  Quote = object(intern_symbol("quote", SPCLFORM));
  Do    = object(intern_symbol("do", SPCLFORM));
  Def   = object(intern_symbol("def", SPCLFORM));
  Put   = object(intern_symbol("put", SPCLFORM));
  If    = object(intern_symbol("if", SPCLFORM));
  Lmb   = object(intern_symbol("lmb", SPCLFORM));
  Amp   = object(intern_symbol("&", SPCLFORM));
}

// main -----------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;
  return 0;
}
