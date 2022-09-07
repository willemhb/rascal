// standard headers -----------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

// renames & __builtins -------------------------------------------------------
#define unreachable __builtin_unreachable
#define popcount    __builtin_popcnt
#define attrs       __attribute__

#define unlikely(x) __builtin_expect((x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

// tag parts & masks ----------------------------------------------------------
#define QNAN  0x7ffc000000000000ul
#define SIGN  0x8000000000000000ul

#define HTAG0 0x0000000000000000ul
#define HTAG1 0x0001000000000000ul
#define HTAG2 0x0002000000000000ul
#define HTAG3 0x0003000000000000ul

#define HTMASK (SIGN|QNAN|HTAG3)
#define WTMASK (HTMASK|0xff00000000ul)
#define OTMASK (HTMASK|7ul)

#define PMASK (~HTMASK)
#define OMASK (PMASK&~7ul)
#define LMASK PMASK
#define IMASK UINT32_MAX
#define UMASK IMASK

#define WTAG0 0x0000000000000000ul
#define WTAG1 0x0000000100000000ul
#define WTAG2 0x0000000200000000ul

#define LTAG0 0x0000000000000000ul
#define LTAG1 0x0000000000000001ul
#define LTAG2 0x0000000000000002ul
#define LTAG3 0x0000000000000003ul
#define LTAG4 0x0000000000000004ul
#define LTAG5 0x0000000000000005ul
#define LTAG6 0x0000000000000006ul
#define LTAG7 0x0000000000000007ul
#define LTAG8 0x0000000000000008ul
#define LTAG9 0x0000000000000009ul
#define LTAGA 0x000000000000000aul
#define LTAGB 0x000000000000000bul
#define LTAGC 0x000000000000000cul
#define LTAGD 0x000000000000000dul
#define LTAGE 0x000000000000000eul
#define LTAGF 0x000000000000000ful

// tags -----------------------------------------------------------------------
#define IMMEDIATE    (QNAN)
#define OBJECT       (SIGN|QNAN)

#define INTEGER      (IMMEDIATE|HTAG0)
#define ARITY        (IMMEDIATE|HTAG1)
#define POINTER      (IMMEDIATE|HTAG2)
#define SMALL        (IMMEDIATE|HTAG3)

#define CHARACTER    (SMALL|WTAG0)
#define BOOLEAN      (SMALL|WTAG1)
#define ESCAPE       (SMALL|WTAG2)

#define PRIMITIVE    (OBJECT|HTAG0)
#define CELL         (OBJECT|HTAG1)
#define TABLE        (OBJECT|HTAG2)
#define BINARY       (OBJECT|HTAG3)

#define SYMBOL       (PRIMITIVE|LTAG0)
#define FUNCTION     (PRIMITIVE|LTAG1)
#define METHODTABLE  (PRIMITIVE|LTAG2)
#define BYTECODE     (PRIMITIVE|LTAG3)
#define PORT         (PRIMITIVE|LTAG4)
#define HEAP         (PRIMITIVE|LTAG5)
#define STACK        (PRIMITIVE|LTAG6)
#define HASHENTRY    (PRIMITIVE|LTAG7)
#define SYMBOLTABLE  (PRIMITIVE|LTAG8)

#define LIST         (CELL|LTAG0)
#define PAIR         (CELL|LTAG1)
#define TUPLE        (CELL|LTAG2)
#define CLOSURE      (CELL|LTAG2)
#define MODULE       (CELL|LTAG3)
#define UPVALUE      (CELL|LTAG4)
#define CONTFRAME    (CELL|LTAG5)
#define ENVIRONMENT  (CELL|LTAG7)

#define SET          (TABLE|LTAG0)
#define DICT         (TABLE|LTAG1)
#define RECORD       (TABLE|LTAG2)
#define VECTOR       (TABLE|LTAG3)
#define SETNODE      (TABLE|LTAG4)
#define DICTNODE     (TABLE|LTAG5)
#define RECORDNODE   (TABLE|LTAG6)
#define VECTORNODE   (TABLE|LTAG7)

#define STRING       (BINARY|LTAG0)
#define INSTRUCTIONS (BINARY|LTAG1)
#define COMPLEX      (BINARY|LTAG2)
#define RATIO        (BINARY|LTAG3)
#define BIGINT       (BINARY|LTAG4)
#define C_VECTOR     (BINARY|LTAG5)
#define C_ARRAY      (BINARY|LTAG6)
#define C_STRUCT     (BINARY|LTAG7)

// typedefs -------------------------------------------------------------------
// utility typedefs -----------------------------------------------------------
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// immediate & general typedefs -----------------------------------------------
typedef uintptr_t value_t;

typedef union
{
  double    fval;
  uintptr_t bits;
  void     *addr;

  struct
  {
    uintptr_t aval : 48;
    uintptr_t      : 16;
  };

  struct
  {
    long lval : 48;
    long      : 16;
  };

  struct
  {
    union
    {
      uint uval;
      int ival;
    };
    
    uint wtag;
  };
} unboxed_t;

// rascal typedefs ------------------------------------------------------------
// immediate typedefs ---------------------------------------------------------
typedef double flonum_t;
typedef long integer_t;
typedef ulong arity_t;
typedef void* pointer_t;
typedef char character_t;
typedef bool boolean_t;

// object typedefs ------------------------------------------------------------
typedef struct cell_t         cell_t;

typedef struct symbol_t       symbol_t;
typedef struct sym_table_t    sym_table_t;

typedef struct heap_t         heap_t;
typedef struct stack_t        stack_t;

typedef struct port_t         port_t;
typedef struct buffer_t       buffer_t;

typedef struct hashentry_t    hashentry_t;

typedef struct function_t     function_t;
typedef struct string_t       string_t;
typedef struct bytecode_t     bytecode_t;
typedef struct methods_t      methods_t;
typedef struct method_t       method_t;

typedef struct table_t        table_t;
typedef struct vector_t       vector_t;
typedef struct dict_t         dict_t;
typedef struct set_t          set_t;
typedef struct node_t         node_t;

typedef cell_t pair_t;
typedef cell_t cons_t;
typedef cell_t list_t;
typedef cell_t environment_t;
typedef cell_t continuation_t;
typedef cell_t closure_t;
typedef cell_t tuple_t;

typedef dict_t record_t;

// function pointer typedefs --------------------------------------------------
typedef void   (*behavior_t)(void);
typedef size_t (*sizefn_t)(value_t x);
typedef ulong  (*hashfn_t)(value_t x);
typedef size_t (*prinfn_t)(value_t x, port_t *ios);
typedef int    (*ordfn_t)(value_t x, value_t y);
typedef void   (*finalfn_t)(value_t x);

// casting and accessor macros ------------------------------------------------
#define fval(v)                      (((unboxed_t)(v)).fval)
#define aval(v)                      (((unboxed_t)(v)).aval)
#define lval(v)                      (((unboxed_t)(v)).lval)
#define ival(v)                      (((unboxed_t)(v)).ival)
#define uval(v)                      (((unboxed_t)(v)).uval)
#define oval(v)                      ((void*)((v)&OMASK))
#define pval(v)                      ((void*)((v)&PMASK))
#define asa(type, cnvt, x)           ((type)cnvt(x))
#define getf(type, x, slot)          (((type##_t*)oval(x))->slot)
#define getf_s(type, x, slot, fname) (to##type(x, fname)->slot)

#define hitag(x) ((x)&HTMASK)
#define obtag(x) ((x)&OTMASK)
#define imtag(x) ((x)&WTMASK)

// globals --------------------------------------------------------------------
// special/common constants ---------------------------------------------------
const value_t TRUE = BOOLEAN|1;
const value_t FALSE = BOOLEAN|0;

const value_t ZERO = INTEGER|0;
const value_t ONE = INTEGER|1;

const value_t NIL = LIST;
const value_t UNBOUND = SYMBOL;
const value_t NONE = PAIR;
const value_t ENVT0 = ENVIRONMENT;

const value_t SET0 = SET;
const value_t DICT0 = DICT;
const value_t VECTOR0 = VECTOR;
const value_t TUPLE0 = TUPLE;

const value_t STRING0 = STRING;

// interpreter state ----------------------------------------------------------
// main registers -------------------------------------------------------------
uint Ctl, Arg;
value_t Fun, Env, Val;
stack_t *Stack, *Eval;

// upvalues cache -------------------------------------------------------------
value_t UpValues;

// memory areas & management --------------------------------------------------
heap_t  *Heap;
stack_t *Final;

// standard streams -----------------------------------------------------------
port_t *Ins, *Outs, *Errs;

// error utilities ------------------------------------------------------------
jmp_buf Safepoint;
value_t Agitant;

size_t prin( value_t x, port_t *ios );

void recover(void)
{
  Agitant = NIL;
}

void vescape(char *fname, value_t agitant, char *fmt, va_list args)
{
  fprintf( stderr, "%s: error: ", fname );
  vfprintf( stderr, fmt, args);

  Agitant = agitant;

  if (agitant != NONE)
    {
      prin( agitant, Errs );
    }

  fprintf( stderr, ".\n" );
  va_end( args );
  longjmp( Safepoint, 1 );
}

void escape(char *fname, value_t agitant, char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  vescape(fname, agitant, fmt, va);
  unreachable();
}

void require(bool test, char *fname, value_t agitant, char *fmt, ...)
{
  va_list va;

  va_start(va, fmt);
  
  if (!test)
    vescape( fname, agitant, fmt, va);

  va_end(va);
}

// describe macros ------------------------------------------------------------
#define describe_safecast(type)			\
  to##type(value_t x, char *fname)		\
  {						\
    require(is##type(x),			\
	    fname,				\
    	    x,					\
	    "expected type %s, got ",		\
	    #type,				\
	    x );				\
    return as##type(x);				\
  }

#define describe_tag_predicate(type, MASK, TAG)		\
  bool is##type(value_t x)				\
  {							\
    return (x&MASK) == TAG;				\
  }

#define describe_tag_predicate_c(type, MASK, TAG)	\
  bool is##type(value_t x)				\
  {							\
    return (x&MASK) != TAG;				\
  }

#define describe_tag_predicate_no_null(type, MASK, TAG, cnvt)	\
  bool is##type(value_t x)					\
  {								\
  return (x&MASK) == TAG && !!cnvt(x);				\
  }

#define describe_value_predicate(type, value)	\
  bool is##type(value_t x)			\
  {						\
    return x == value;				\
  }

// immediate typedefs & macros ------------------------------------------------
#define asflonum(x) asa(flonum_t, fval, x)
#define asint(x)    asa(integer_t, lval, x)
#define asarity(x)  asa(arity_t, aval, x)
#define asptr(x)    asa(pointer_t, pval, x)
#define aschar(x)   asa(character_t, ival, x)
#define asbool(x)   asa(boolean_t, ival, x)

describe_tag_predicate(immediate, OBJECT, IMMEDIATE)
describe_tag_predicate(object, OBJECT, OBJECT)
describe_tag_predicate_c(flonum, QNAN, QNAN)
describe_tag_predicate(int, HTMASK, INTEGER)
describe_tag_predicate(arity, HTMASK, ARITY)
describe_tag_predicate(ptr, HTMASK, POINTER)
describe_tag_predicate(small, HTMASK, SMALL)
describe_tag_predicate(bool, WTMASK, BOOLEAN)
describe_tag_predicate(char, WTMASK, CHARACTER)

describe_value_predicate(true, TRUE)
describe_value_predicate(false, FALSE)

flonum_t describe_safecast(flonum)
integer_t describe_safecast(int)
arity_t describe_safecast(arity)
pointer_t describe_safecast(ptr)
character_t describe_safecast(char)
boolean_t describe_safecast(bool)

value_t flonum( flonum_t f )
{
  return ((unboxed_t)f).bits;
}

value_t integer( integer_t i )
{
  return ((value_t)i & PMASK) | INTEGER;
}

value_t arity( arity_t a )
{
  return (a & PMASK) | ARITY;
}

value_t boolean( integer_t i )
{
  return i ? TRUE : FALSE;
}

boolean_t Cbool( value_t x )
{
  return (x == FALSE || x == NIL) ? false : true;
}

value_t character( integer_t i )
{
  return ((value_t)i & IMASK) | CHARACTER;
}

// object typedefs & utility macros -------------------------------------------
// internal types -------------------------------------------------------------
// stack type -----------------------------------------------------------------
struct stack_t
{
  uint sp, cap;
  value_t *data;
};

#define N_STACK 128

#define asstk(x)        asa(stack_t*, oval, x)

#define stksp(x)        getf(stack, x, sp)
#define stkcap(x)       getf(stack, x, cap)
#define stkdata(x)      getf(stack, x, data)

#define sref(x, n)      (stkdata(x)[(n)])
#define speek(x, n)     (stkdata(x)[stksp(x)-(n)-1])

#define stksp_s(x, f)   getf_s(stack, x, sp, f)
#define stkcap_s(x, f)  getf_s(stack, x, cap, f)
#define stkdata_s(x, f) getf_s(stack, x, data, f)

describe_tag_predicate(stk, OTMASK, STACK)

stack_t *describe_safecast(stk)

bool check_stk_grow(stack_t *s, arity_t n)
{
  return s->sp + n >= s->cap;
}

bool check_stk_shrink(stack_t *s)
{
  return s->cap > N_STACK && s->sp <= s->cap / 2;
}

void init_stk(stack_t *s)
{
  s->sp   = 0;
  s->cap  = N_STACK;
  s->data = calloc( s->cap, sizeof(value_t) );
}

void finalize_stk(stack_t *s)
{
  free(s->data);
}

void grow_stk(stack_t *s)
{
  s->cap *= 2;
  s->data = realloc( s->data, s->cap * sizeof(value_t) );
}

void shrink_stk(stack_t *s)
{
  s->cap /= 2;
  s->data = realloc( s->data, s->cap * sizeof(value_t) );
}

arity_t stk_push(stack_t *s, value_t x)
{
  if (check_stk_grow(s, 1))
    grow_stk(s);

  arity_t out = s->sp++;
  s->data[out] = x;
  
  return out;
}

value_t stk_pop(stack_t *s)

{
  assert(s->sp);

  value_t out = s->data[--s->sp];

  if (check_stk_shrink(s))
    shrink_stk(s);

  return out;
}

// heap type ------------------------------------------------------------------
#define N_WORD 16384
#define N_ALIGN 8192
#define RESIZE_F 0.625
#define GROW_F 2.0
#define HEAP_W sizeof(value_t)
#define HEAP_A HEAP_W*2

struct heap_t
{
  size_t align, word;
  size_t n_bytes, bytes_used;
  size_t n_words, words_used;
  size_t n_align, align_used;

  bool grow, grew, collecting, young;

  uchar *space, *space_map, *swap, *swap_map;
};

void init_heap(heap_t *h)
{
  h->word       = HEAP_W;
  h->align      = HEAP_A;
  
  h->n_align    = N_ALIGN;
  h->n_words    = N_WORD;
  h->n_bytes    = h->n_words * h->word;

  h->bytes_used = 0;
  h->words_used = 0;
  h->align_used = 0;
  
  h->grow       = false;
  h->grew       = false;
  h->collecting = false;
  h->young      = true;

  h->space      = malloc( h->n_bytes );
  h->space_map  = malloc( h->n_align );
  h->swap       = malloc( h->n_bytes );
  h->swap_map   = malloc( h->n_align );
}

// cell types -----------------------------------------------------------------
struct cell_t
{
  value_t car;
  value_t cdr;
};

describe_tag_predicate(cell, HTMASK, CELL)
describe_tag_predicate(pair, OTMASK, PAIR)
describe_tag_predicate(list, OTMASK, LIST)
describe_value_predicate(nil, NIL)
describe_tag_predicate_no_null(cons, OTMASK, LIST, oval)
describe_tag_predicate(envt, OTMASK, ENVIRONMENT)
describe_tag_predicate(cont, OTMASK, CONTFRAME)
describe_tag_predicate(closure, OTMASK, CLOSURE)
describe_tag_predicate(tuple, OTMASK, TUPLE)

#define ascell(x)      asa(cell_t*, oval, x)
#define aspair(x)      asa(pair_t*, oval, x)
#define ascons(x)      asa(cons_t*, oval, x)
#define aslist(x)      asa(list_t*, oval, x)
#define asenvt(x)      asa(environment_t*, oval, x)
#define ascont(x)      asa(continuation_t*, oval, x)
#define asclosure(x)   asa(closure_t*, oval, x)
#define astuple(x)     asa(tuple_t*, oval, x)

cell_t         *describe_safecast(cell)
pair_t         *describe_safecast(pair)
cons_t         *describe_safecast(cons)
list_t         *describe_safecast(list)
environment_t  *describe_safecast(envt)
continuation_t *describe_safecast(cont)
closure_t      *describe_safecast(closure)
tuple_t        *describe_safecast(tuple)

#define car(x)         getf(cell, x, car)
#define cdr(x)         getf(cell, x, cdr)

#define car_s(x, f)    getf_s(cell, x, car, f)
#define cdr_s(x, f)    getf_s(cell, x, cdr, f)

#define first(x)       getf(pair, x, car)
#define second(x)      getf(pair, x, cdr)

#define first_s(x, f)  getf_s(pair, x, car, f)
#define second_s(x, f) getf_s(pair, x, cdr, f)

#define head(x)        getf(cons, x, car)
#define tail(x)        getf(cons, x, cdr)

#define head_s(x, f)   getf_s(cons, x, car, f)
#define tail_s(x, f)   getf_s(cons, x, cdr, f)

#define locals(x)      getf(environment, x, car)
#define parent(x)      getf(environment, x, cdr)

#define locals_s(x, f) getf_s(environment, x, car, f)
#define parent_s(x, f) getf_s(environment, x, car, f)

#define stack(x)       getf(continuation, x, car)
#define values(x)      getf(continuation, x, cdr)

#define stack_s(x, f)  getf_s(continuation, x, car, f)
#define values_s(x, f) getf_s(continuation, x, cdr, f)

#define clfunc(x)      getf(closure, x, car)
#define clenvt(x)      getf(closure, x, cdr)

#define clfunc_s(x, f) getf_s(closure, x, car, f)
#define clenvt_s(x, f) getf_s(closure, x, cdr, f)

#define tuplen(x)      getf(tuple, x, car)
#define tuparity(x)    (((unboxed_t)tuplen(x)).aval)
#define tupvals(x)     (&getf(tuple, x, cdr))

// generic length/arity -------------------------------------------------------
#define length(x) car(x)
#define arity(x) (((unboxed_t)car(x)).aval)

struct symbol_t
{
  value_t idno;
  value_t props;
  value_t length;
  ulong   hash;
  char    name[0];
};

struct sym_table_t
{
  value_t  counter;

  size_t   length;
  size_t   cap;

  char   **keys;
  long    *orders;
  ulong   *hashes;
  value_t *symbols;
};

struct addr_table_t
{
  size_t   length;
  size_t   cap;

  void   **keys;
  long    *orders;
  ulong   *hashes;
  value_t *binds;
};

#define assym(x)        asa(symbol_t*, oval, x)

#define idno(x)         getf(symbol, x, idno)
#define symname(x)      getf(symbol, x, name)

#define idno_s(x, f)    getf_s(symbol, x, idno, f)
#define symname_s(x, f) getf_s(symbol, x, name, f)

describe_tag_predicate(sym, OTMASK, SYMBOL)
describe_value_predicate(none, NONE)

symbol_t *describe_safecast(sym)

// binary types ---------------------------------------------------------------
struct string_t
{
  value_t length;
  char    chars[0];
};

#define asstr(x)       asa(string_t*, oval, x)

#define slen(x)        getf(string, x, length)
#define schars(x)      getf(string, x, chars)

#define slen_s(x, f)   getf_s(string, x, length, f)
#define schars_s(x, f) getf_s(string, x, chars, f)

describe_tag_predicate(str, OTMASK, STRING)
describe_value_predicate(str0, STRING0)

string_t *describe_safecast(str)

char strref(string_t *s, arity_t n)
{
  assert(n < aval(s->length) );
  return s->chars[n];
}



// function implementation(s) -------------------------------------------------
struct function_t
{
  value_t name;      // function name
  value_t names;     // local function namespace
  value_t module;    // global function namespace
  value_t props;     // metadata
  value_t signature; // function argument spec
  value_t template;  // function "body"
};

struct bytecode_t
{
  value_t  length;
  tuple_t *values;
  short    instr[0];
};

struct methods_t
{
  value_t fmethods; // multi-key dictionary of fixed arity methods
  value_t vmethods; // multi-key dictionary of variable arity methods
};

#define asfn(x)      asa(function_t*, oval, x)
#define ascode(x)    asa(bytecode_t*, oval, x)

#define fnname(x)     getf(function, x, name)
#define fnnames(x)    getf(function, x, names)
#define fnmodule(x)   getf(function, x, module)
#define fnprops(x)    getf(function, x, props)
#define fnsig(x)      getf(function, x, signature)
#define fntemplate(x) getf(function, x, template)

#define bcinstr(x)    getf(bytecode, x, instr)
#define bcvals(x)     getf(bytecode, x, vals)
#define bclen(x)      getf(bytecode, x, length)
#define bcarity(x)    (((unboxed_t)bclen(x)).aval)

describe_tag_predicate(fn, OTMASK, FUNCTION)


// port implementation --------------------------------------------------------
#define N_BUFFER 128

struct buffer_t
{
  uint bufi, bufc;
  char *chars;
};

struct port_t
{
  value_t val, pval;

  FILE *stream;
  buffer_t *buffer;

  uint tok, ptok;
  uint line, col;
  ulong flags, pos;
};

void init_buffer( buffer_t *b )
{
  b->bufi  = 0;
  b->bufc  = N_BUFFER;
  b->chars = malloc(b->bufc);
}

void grow_buffer( buffer_t *b )
{
  b->bufc *= 2;
  b->chars = realloc(b->chars, b->bufc);
}

void shrink_buffer( buffer_t *b )
{
  b->bufc /= 2;
  b->chars = realloc(b->chars, b->bufc);
}

void init_port( port_t *p )
{
  p->val  = NIL;
  p->pval = NIL;
}

// table types ----------------------------------------------------------------
struct node_t
{
  uint    bitmap;
  uchar   depth;
  uchar   length;
  ushort  tag;

  value_t space[0];
};

struct hashentry_t
{
  union
  {
    value_t location;
    value_t locations;
    value_t child;
  };

  ulong   hash;
};

struct table_t
{
  value_t length;
  value_t signature;
  value_t cache;
  value_t data;
};

struct vector_t
{
  table_t base;
};

struct dict_t
{
  table_t base;
  value_t keyvals;
  value_t compare;
};

struct set_t
{
  table_t base;
  value_t values;
  value_t compare;
};

// utility functions ----------------------------------------------------------

// read implementation --------------------------------------------------------
value_t read(port_t *ios);

// print implementation -------------------------------------------------------
size_t  prin(value_t x, port_t *ios);

// exec implementation --------------------------------------------------------
value_t exec(value_t closure);

// compile implementation -----------------------------------------------------
int     compile_file(char *fname);
value_t load(char *fname);
value_t compile(value_t form);

// initialization -------------------------------------------------------------
// backing objects ------------------------------------------------------------
stack_t StackObject, EvalObject, FinalObject;

heap_t HeapObject;

port_t InsObject, OutsObject, ErrsObject;

void stacks_init(void)
{
  Stack = &StackObject;
  Eval  = &EvalObject;
  Final = &FinalObject;

  init_stk(Stack);
  init_stk(Eval);
  init_stk(Final);
}

void heaps_init(void)
{
  Heap = &HeapObject;

  init_heap(Heap);
}

void streams_init(void)
{
  Ins = &InsObject;
  Outs = &OutsObject;
  Errs = &ErrsObject;

  init_port(Ins);
  init_port(Outs);
  init_port(Errs);
}

// entry point ----------------------------------------------------------------
int main(void)
{
  return 0;
}
