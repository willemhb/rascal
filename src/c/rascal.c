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
typedef uword value_t;                    // tagged value

typedef double             number_t;
typedef struct object      object_t;
typedef struct type        type_t;
typedef struct symbol      symbol_t;
typedef struct cons        cons_t;
typedef struct list        list_t;
typedef struct function    function_t;

typedef enum {
  NOTYPE,
  UNIT,
  NUMBER,
  OBJECT
} value_type_t;

typedef enum {
  BOTTOM,
  DATA,
  UNION,
  TOP
} kind_t;

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

  // binary flags
  ENCODED =0x0010,

  // function flags
  NATIVE  =0x0001,
  LAMBDA  =0x0002,
  VARIADIC=0x0004,
} objfl_t;

struct object {
  object_t* next;   // presently unused

  union {
    type_t* type;
    struct {
      uword typeBits : 48;
      uword flags    : 16;
    };
    uword headBits;
  };
};

#define HEADER object_t obj

struct type {
  HEADER;
  type_t *left, * right; // union members
  char* name;
  usize size;
  uint64 idno;
  value_type_t value_type;
  kind_t kind;
};

struct symbol {
  HEADER;
  char* name;
  value_t bind; // toplevel
  uint64 idno;
  symbol_t* left, * right;
};

struct cons {
  HEADER;
  value_t car, cdr;
};

struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

struct function {
  HEADER;
  symbol_t* name;
  usize     arity;
  type_t*   type;
  union {
    value_t (*native)(usize n, value_t* args); // native
    cons_t* closure;
  };
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

// type objects
uint64 TypeCounter = 9;

type_t TypeType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY,
  },
  .name      ="type",
  .size      =sizeof(type_t),
  .idno      =1,
  .value_type=OBJECT,
  .kind      =DATA
};

type_t SymbolType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY,
  },
  .name      ="symbol",
  .size      =sizeof(symbol_t),
  .idno      =2,
  .value_type=OBJECT,
  .kind      =DATA
};

type_t ConsType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY,
  },
  .name      ="cons",
  .size      =sizeof(cons_t),
  .idno      =3,
  .value_type=OBJECT,
  .kind      =DATA
};

type_t ListType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="list",
  .size      =sizeof(list_t),
  .idno      =4,
  .value_type=OBJECT,
  .kind      =DATA
};

type_t FunctionType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="function",
  .size      =sizeof(function_t),
  .idno      =5,
  .value_type=OBJECT,
  .kind      =DATA
};

type_t NumberType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="number",
  .size      =sizeof(number_t),
  .idno      =6,
  .value_type=NUMBER,
  .kind      =DATA 
};

type_t UnitType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="unit",
  .size      =sizeof(value_t),
  .idno      =7,
  .value_type=UNIT,
  .kind      =DATA   
};

type_t NoneType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="none",
  .size      =0,
  .idno      =8,
  .value_type=NOTYPE,
  .kind      =BOTTOM 
};

type_t AnyType = {
  .obj       ={
    .type =&TypeType,
    .flags=NOFREE|GRAY
  },
  .name      ="any",
  .size      =0,
  .idno      =9,
  .value_type=NOTYPE,
  .kind      =TOP
};

// other global singletons
list_t EmptyList = {
  .obj = {
    .type =&ListType,
    .flags=NOFREE|GRAY,
  },
  .arity=0,
  .head =NIL,
  .tail =&EmptyList
};

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

// data -----------------------------------------------------------------------
void* as_pointer( value_t x ) {
  return (void*)(x & VALMASK);
}

object_t* as_object( value_t x ) {
  return as_pointer(x);
}

number_t as_number( value_t x ) {
  return ((ieee64_t)x).dbl;
}

type_t* type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case NILTAG: return &UnitType;
    case OBJTAG: return (type_t*)(as_object(x)->headBits & VALMASK);
    default:     return &NumberType;
  }
}

usize size_of( value_t x ) {
  return type_of(x)->size;
}

bool has_type( value_t x, type_t* type ) {
  if ( type == NULL )
    return false;

  switch ( type->kind ) {
    case BOTTOM: return false;
    case DATA:   return type_of(x) == type;
    case UNION:  return has_type(x, type->left) || has_type(x, type->right);
    case TOP:    return true;
  }

  unreachable();
}

// object ---------------------------------------------------------------------
bool has_flag( void* obj, flags fl );

value_t object( void* obj ) {
  return ((value_t)obj) | OBJTAG;
}

void init_object( void* slf, type_t* type, flags fl ) {
  object_t* obj = slf;

  obj->next =NULL;
  obj->type =type;
  obj->flags=fl|GRAY;
}

void* make_object( type_t* type, flags fl ) {
  object_t* out = allocate(type->size);
  init_object(out, type, fl);
  return out;
}

// symbol ---------------------------------------------------------------------
symbol_t* as_symbol( value_t x ) {
  return as_pointer(x);
}

bool is_symbol( value_t x ) {
  return has_type(x, &SymbolType);
}

bool is_keyword( value_t x ) {
  return is_symbol(x) && has_flag(as_symbol(x), LITERAL);
}



static symbol_t** locate_symbol( char* name, symbol_t** root) {
  while ( *root ) {
    int o = strcmp(name, (*root)->name);

    if ( o < 0 )
      root = &(*root)->left;

    else if ( o > 0 )
      root = &(*root)->right;

    else
      break;
  }

  return root;
}

static symbol_t* make_symbol( char* name, bool interned ) {
  symbol_t* out = make_object(&SymbolType, INTERNED*interned|LITERAL*(*name == ':'));
  out->left = out->right = NULL;
  out->idno = ++SymbolCounter;
  out->bind = UNDEFINED;

  return out;
}

static symbol_t* intern_symbol( char* name ) {
  symbol_t** location = locate_symbol(name, &SymbolTable);

  if ( *location == NULL )
    *location = make_symbol(name, true);

  return *location;
}

symbol_t* symbol( char* name, bool intern ) {
  if ( intern )
    return intern_symbol(name);

  if ( name == NULL )
    name = "symbol";

  return make_symbol(name, false);
}

// cons -----------------------------------------------------------------------
cons_t* cons( value_t ca, value_t cd );

// list -----------------------------------------------------------------------
list_t* as_list( value_t x ) {
  return as_pointer(x);
}

bool is_list( value_t x ) {
  return has_type(x, &ListType );
}

list_t* list( value_t h, list_t* t );

// function -------------------------------------------------------------------
function_t* as_function( value_t x ) {
  return as_pointer(x);
}

bool is_function( value_t x ) {
  return has_type(x, &FunctionType);
}

bool is_native( value_t x ) {
  return is_function(x) && has_flag(as_function(x), NATIVE);
}

bool is_closure( value_t x ) {
  return is_function(x) && has_flag(as_function(x), LAMBDA);
}

function_t* function( symbol_t* name, type_t* type, usize arity, bool vargs, value_t (*native)(usize n, value_t* args), cons_t* closure );

// lang -----------------------------------------------------------------------
static bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return !!(as_object(x)->flags && LITERAL);

  if ( has_type(x, &ListType) )
    return as_list(x)->arity > 0;

  return true;
}

static value_t   lookup( value_t x, list_t* env );
static object_t* define( value_t x, list_t* env );
static object_t* resolve( value_t x, list_t* env );

// special forms --------------------------------------------------------------
value_t Quote, Do, Def, Put, If, Lmb, Amp;

value_t eval( value_t x, list_t* env );

value_t eval( value_t x, list_t* env ) {
  value_t v, h, b;
  symbol_t* n;
  cons_t* p;
  list_t* a, *s, *f, *e;
  function_t* fn;
  bool isva;
  usize nargs;

 eval_top:
  if ( is_literal(x) )
    v = x;
  
  else if ( is_symbol(x) ) {
      v = lookup(x, env);
      require(v != UNDEFINED, x, "undefined symbol %s", as_symbol(x)->name);
    }

  else {
    list_t* form = as_list(x);
    value_t head = form->head;
    list_t* args = form->tail;

    if ( is_symbol(head) && has_flag(as_symbol(head), SPCLFORM) ) {
      if ( head == Quote ) {
        require(args->arity == 1, x, "wrong number of expressions in quote");
        v = args->head;
      } else if ( head == Do ) {
        require( args->arity >= 1, x, "not enough expressions in do");

        while ( args->arity > 1 ) {
          eval(args->head, env);
          args = args->tail;
        }

        x = args->head;
        goto eval_top;

      } else if ( head == Def ) {
        require( args->arity == 2, x, "wrong number of expressions in def");
        h = args->head;
        b = args->tail->head;
        require(is_symbol(h), x, "name has wrong type in def");
        n = as_symbol(h);
        require(!has_flag(n, LITERAL), x, "binding to keyword in def");
        require(env->arity > 0 || !has_flag(n, CONSTANT), x, "binding to constant symbol at toplevel in def");
        p = (cons_t*)define(h, env);
        v = eval(b, env);
        p->cdr = v;
        if ( is_function(v) )
          as_function(v)->name = n;
      } else if ( head == Put ) {
        require(args->arity == 2, x, "wrong number of expressions in put");
        h = args->head;
        b = args->tail->head;
        require(is_symbol(h), x, "name has wrong type in put");
        n = as_symbol(h);
        require(!has_flag(n, LITERAL), x, "binding to keyword in put");
        require(env->arity > 0 || !has_flag(n, CONSTANT), x, "assigning to constant symbol at toplevel in put");
        p = (cons_t*)resolve(h, env);
        require(p != NULL, x, "trying to assign to name before its definition in put");
        v = eval(b, env);
        p->cdr = v;
        if ( is_function(v) )
          as_function(v)->name = n;
      } else if ( head == If ) {
        require(args->arity >= 2, x, "not enough expressions in if");
        while ( args->arity > 1 ) {
          if (eval(args->head, env) != NIL) {
            args = args->tail;
            break;
          }
          args = args->tail->tail;
        }
        x = args->head;
        goto eval_top;
      } else if ( head == Lmb ) {
        require(args->arity >= 2, x, "not enough expressions in lmb");
        require(is_list(args->head), x, "malformed formal parameters in lmb");
        a = as_list(args->head);
        s = args->tail;
        f = &EmptyList;
        isva = false;
        while ( a->arity ) {
          require(is_symbol(a->head), x, "malformed formal parameters in lmb");
          if ( a->head == Amp ) {
            require(a->arity == 2, x, "malformed formal parameters in lmb");
            isva = true;
          } else {
            f = list(a->head, f);
          }
          a = a->tail;
        }
        p = cons(object(f), object(s));
        p = cons(object(p), object(env));
        v = object(function(as_symbol(Lmb), NULL, f->arity, isva, NULL, p));
      } else {
        error(x, "unkown special form '%s'", as_symbol(head)->name);
      }
    } else {
      head = eval(head, env);
      require(is_function(head), x, "operator is not a function");
      fn = as_function(head);
      nargs = args->arity;
      argco(fn, x, nargs);
      isva = has_flag(fn, VARIADIC);

      if ( has_flag(fn, NATIVE) ) {
        value_t argarr[nargs];

        for (usize i=0; i<nargs; i++, args=args->tail)
          argarr[i] = eval(args->head, env);

        v = fn->native(nargs, argarr);
      } else {
        
      }
    }
  }

  return v;
}

// main -----------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;
  return 0;
}
