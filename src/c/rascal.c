#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// typedefs +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// utility types --------------------------------------------------------------
typedef union {
  uintptr_t word;
  double real;
  struct {
    uintptr_t fraction : 52;
    uintptr_t exponent : 11;
    uintptr_t sign     :  1;
  };
} ieee754_64_t;

// internal enum types --------------------------------------------------------
typedef enum type_t {
  NOTYPE,
  NUMBER,
  UNIT,
  NATIVE,
  ENVIRONMENT,
  SYMBOL,
  LIST
} type_t;

// object types (forward) -----------------------------------------------------
typedef struct object      object_t;
typedef struct native      native_t;
typedef struct environment environment_t;
typedef struct symbol      symbol_t;
typedef struct list        list_t;

// value types ----------------------------------------------------------------
typedef uintptr_t value_t;
typedef double number_t;

struct object {
  object_t* next;
  type_t    type;
  short     gray;
  short     black;
};

// other internal types -------------------------------------------------------
typedef value_t (*native_callback_t)(list_t* form, environment_t* environment, list_t* args);

// object types ---------------------------------------------------------------
struct native {
  object_t          obj;
  symbol_t*         name;
  bool              special;
  bool              variadic;
  size_t            arity;
  native_callback_t callback;
};

struct environment {
  object_t       obj;
  list_t*        names;
  list_t*        binds;
  environment_t* parent;
};

struct symbol {
  object_t obj;
  symbol_t* left, * right;
  char* name;
  value_t bind;
};

struct list {
  object_t obj;
  size_t arity;
  value_t head;
  list_t* tail;
};

// internal structure types ---------------------------------------------------
// array types ----------------------------------------------------------------
typedef struct {
  value_t* data;
  size_t cnt, cap;
} values_t;

typedef struct {
  object_t** data;
  size_t cnt, cap;
} objects_t;

typedef struct {
  char* data;
  size_t cnt, cap;
} buffer_t;

// vm type --------------------------------------------------------------------
typedef struct {
  // interpreter --------------------------------------------------------------
  struct {
    values_t stack; // temporary values store ---------------------------------
  } interpreter;

  // reader -------------------------------------------------------------------
  struct {
    values_t subexpressions;
    buffer_t buffer;
  } reader;

  // heap ---------------------------------------------------------------------
  struct {
    objects_t grays;   // marked objects awaiting tracing
    object_t* live;    // linked list of live objects
    size_t used, max;  // current heap size + next collection
  } heap;

  // symbol table -------------------------------------------------------------
  struct {
    symbol_t* root;
  } symbol_table;

  // error --------------------------------------------------------------------
  struct {
    jmp_buf jmpbuf;
  } error;
} vm_t;

// tags/masks +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define QNAN 0x7ff8000000000000UL
#define SIGN 0x8000000000000000UL
#define TAGM 0xffff000000000000UL
#define VALM 0x0000ffffffffffffUL

#define NUL  0x7ffc000000000000UL
#define OBJ  0x7ffd000000000000UL

#define NOTHING (NUL|1UL) // invalid value

// size limits ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAXPOW2  0x8000000000000000UL
#define MAXSIZE  0x0000ffffffffffffUL

// other size macros ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define HEAPSIZE 0x0000000000010000UL

// forward declarations +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// runtime implementations ----------------------------------------------------
void* allocate(size_t n, bool fromHeap);
void* allocate_array(size_t n, size_t o, bool fromHeap);
void* duplicate(void* p, size_t n, bool fromHeap);
void* duplicate_array(void* p, size_t n, size_t o, bool fromHeap);
char* duplicate_string(const char* string, bool fromHeap);
void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap);
void* reallocate_array(void* ptr, size_t oldN, size_t newN, size_t o, bool fromHeap);
void  deallocate(void* ptr, size_t n, bool fromHeap);
void  deallocate_array(void* ptr, size_t n, size_t o, bool fromHeap);

// stack API ------------------------------------------------------------------
void     init_interpreter(void);
void     free_interpreter(void);
void     reset_interpreter(void);
size_t   push(value_t val);
size_t   pushn(size_t n, ...);
void     setsp(size_t n);
value_t  pop(void);
value_t  popn(size_t n);
value_t* peek(size_t n);

// type implementations -------------------------------------------------------
// object 'type' --------------------------------------------------------------
object_t* to_object(list_t* form, const char* fname, value_t value);
void*     mk_object(type_t type);
void      init_object(void* ptr, type_t type);
bool      is_object(value_t val);

// number type ----------------------------------------------------------------
number_t to_number(list_t* form, const char* fname, value_t value);
value_t  mk_number(number_t number);
bool     is_number(value_t x);

// unit type ------------------------------------------------------------------
bool is_unit(value_t x);

// native type ----------------------------------------------------------------
native_t* to_native(list_t* form, const char* fname, value_t value);
value_t   mk_native(symbol_t* name, bool special, bool variadic, size_t arity, native_callback_t callback);
bool      is_native(value_t x);
value_t   def_native(const char* fname, bool special, bool variadic, size_t arity, native_callback_t callback);

// environment type -----------------------------------------------------------
environment_t* to_environment(list_t* form, const char* fname, value_t value);
environment_t* mk_environment(list_t* names, list_t* binds, environment_t* parent);
bool           is_environment(value_t x);
value_t        lookup_name(value_t name, environment_t* environment);
value_t        define_name(value_t name, value_t value, environment_t* environment);
value_t        assign_name(value_t name, value_t value, environment_t* environment);

// symbol type ----------------------------------------------------------------
symbol_t* to_symbol(list_t* form, const char* fname, value_t value);
symbol_t* intern_symbol(const char* token);
value_t   mk_symbol(const char* token);
bool      is_symbol(value_t x);
value_t   def_toplevel(const char* name, value_t bind);

// list type ------------------------------------------------------------------
list_t* to_list(list_t* form, const char* fname, value_t value);
value_t mk_list(value_t head, list_t* tail);
bool    is_list(value_t x);
value_t mk_list2(value_t fst, value_t snd);
value_t mk_listn(size_t n, value_t* args);
value_t list_nth(const char* fname, list_t* xs, size_t n);

// native functions -----------------------------------------------------------
// special forms --------------------------------------------------------------
value_t native_quote(list_t* form, environment_t* environment, list_t* args);
value_t native_do(list_t* form, environment_t* environment, list_t* args);
value_t native_if(list_t* form, environment_t* environment, list_t* args);
value_t native_def(list_t* form, environment_t* environment, list_t* args);
value_t native_put(list_t* form, environment_t* environment, list_t* args);

// arithmetic -----------------------------------------------------------------
value_t native_add(list_t* form, environment_t* environment, list_t* args);
value_t native_sub(list_t* form, environment_t* environment, list_t* args);
value_t native_mul(list_t* form, environment_t* environment, list_t* args);
value_t native_div(list_t* form, environment_t* environment, list_t* args);
value_t native_eql(list_t* form, environment_t* environment, list_t* args);
value_t native_not(list_t* form, environment_t* environment, list_t* args);
value_t native_head(list_t* form, environment_t* environment, list_t* args);
value_t native_tail(list_t* form, environment_t* environment, list_t* args);
value_t native_cons(list_t* form, environment_t* environment, list_t* args);
value_t native_len(list_t* form, environment_t* environment, list_t* args);
value_t native_numberp(list_t* form, environment_t* environment, list_t* args);
value_t native_nativep(list_t* form, environment_t* environment, list_t* args);
value_t native_symbolp(list_t* form, environment_t* environment, list_t* args);
value_t native_listp(list_t* form, environment_t* environment, list_t* args);
value_t native_consp(list_t* form, environment_t* environment, list_t* args);
value_t native_emptyp(list_t* form, environment_t* environment, list_t* args);
value_t native_nulp(list_t* form, environment_t* environment, list_t* args);

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v);

// interpreter functions ------------------------------------------------------
// interpreter helpers --------------------------------------------------------
list_t* eval_args(list_t* args, environment_t* environment);
void    argcount(list_t* form, const char* fname, bool variadic, size_t expect, size_t got);
void    argtype(value_t got, list_t* form, const char* fname, type_t expect);

// main interpreter functions -------------------------------------------------
void    rl_error(value_t cause, list_t* form, const char* fname, const char* fmt, ...);
value_t rl_read(void);
value_t rl_eval(value_t v, environment_t* e);
void    rl_print(value_t v);
void    rl_println(value_t v);
void    rl_repl(void);

// utility macros & statics +++++++++++++++++++++++++++++++++++++++++++++++++++
#define rl_require(test, cause, form, fname, message, ...)              \
  do {                                                                  \
    if (!(test))                                                        \
      rl_error(cause, form, fname, message __VA_OPT__(,) __VA_ARGS__);  \
  } while (false)

#define newline() printf("\n")

#define savepoint() setjmp(Vm.error.jmpbuf)

static inline number_t as_number(value_t val) {
  return ((ieee754_64_t)val).real;
}

static inline void* as_pointer(value_t val) {
  return (void*)(val & VALM);
}

static inline object_t* as_object(value_t val) {
  return (object_t*)(val & VALM);
}

static inline value_t tag_pointer(void* p, value_t t) {
  return (value_t)p | t;
}

static const char* type_name(type_t type) {
  const char* out;

  switch ( type ) {
    case NUMBER:      out = "number";      break;
    case UNIT:        out = "unit";        break;
    case NATIVE:      out = "native";      break;
    case ENVIRONMENT: out = "environment"; break;
    case SYMBOL:      out = "symbol";      break;
    case LIST:        out = "list";        break;
    default:          out = "unknown";     break;
  }

  return out;
}

static size_t type_size(type_t type) {
  size_t out;

  switch ( type ) {
    case NUMBER:      out = sizeof(number_t);      break;
    case UNIT:        out = sizeof(value_t);       break;
    case NATIVE:      out = sizeof(native_t);      break;
    case ENVIRONMENT: out = sizeof(environment_t); break;
    case SYMBOL:      out = sizeof(symbol_t);      break;
    case LIST:        out = sizeof(list_t);        break;
    default:          out = 0;                     break;
  }

  return out;
}

// misc utilities +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uint64_t ceilpow2(uint64_t i) {
  if ( i == 0 )
    return 1;

  if ( (i & (i - 1)) == 0 )
    return i;

  if ( i & MAXPOW2 )
    return MAXPOW2;

  while ( i & (i - 1) )
    i = i & (i - 1);

  return i << 1;
}

// describe macros ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SAFECAST(ctype, type, convert)                              \
  ctype to_##type(list_t* form, const char* fname, value_t val) {   \
    rl_require(is_##type(val),                                      \
               val,                                                 \
               form,                                                \
               fname,                                               \
               "expected a value of type %s",                       \
               #type);                                              \
    return (ctype)convert(val);                                     \
  }

#define TYPEP(type, T)                          \
  bool is_##type(value_t val) {                 \
    return rl_type_of(val) == T;                \
  }

#define ARRAY_API(type, ctype, eltype)                                 \
  void init_##type(ctype* array);                                      \
  void free_##type(ctype* array);                                      \
  void reset_##type(ctype* array);                                     \
  size_t resize_##type(ctype* array, size_t n);                        \
  size_t type##_push(ctype* array, eltype element);                    \
  size_t type##_pushn(ctype* array, size_t n, ...);                    \
  size_t type##_write(ctype* array, size_t n, eltype* elements);       \
  eltype type##_pop(ctype* array);                                     \
  eltype type##_popn(ctype* array, size_t n)

#define ARRAY_IMPL(type, ctype, eltype, vatype, encoded)                \
  void init_##type(ctype* array) {                                      \
    array->cnt  = 0;                                                    \
    array->cap  = 0;                                                    \
    array->data = NULL;                                                 \
  }                                                                     \
  void free_##type(ctype* array) {                                      \
    deallocate(array->data, 0, false);                                  \
  }                                                                     \
  void reset_##type(ctype* array) {                                     \
    free_##type(array);                                                 \
    init_##type(array);                                                 \
  }                                                                     \
  size_t resize_##type(ctype* array, size_t n) {                        \
    rl_require(n <= MAXSIZE,                                            \
               NOTHING,                                                 \
               NULL,                                                    \
               "<runtime @ resize_"#type">",                            \
               "overflow" );                                            \
    if (n == 0)                                                         \
      reset_##type(array);                                              \
    else {                                                              \
      size_t padded = n + encoded;                                      \
      size_t newc   = ceilpow2(padded);                                 \
      if (newc != array->cap) {                                         \
        size_t oldc  = array->cap;                                      \
        size_t els   = sizeof(eltype);                                  \
        eltype* data = array->data;                                     \
        array->data  = reallocate_array(data, oldc, newc, els, false);  \
        array->cap   = newc;                                            \
      }                                                                 \
      array->cnt = n;                                                   \
    }                                                                   \
    return array->cnt;                                                  \
  }                                                                     \
  size_t type##_push(ctype* array, eltype element) {                    \
    resize_##type(array, array->cnt+1);                                 \
    array->data[array->cnt-1] = element;                                \
    return array->cnt;                                                  \
  }                                                                     \
  size_t type##_pushn(ctype* array, size_t n, ...) {                    \
    size_t offset = array->cnt;                                         \
    resize_##type(array, array->cnt+n);                                 \
    eltype* buffer = array->data+offset;                                \
    va_list va;                                                         \
    va_start(va, n);                                                    \
    for ( size_t i=0; i<n; i++ )                                        \
      buffer[i] = va_arg(va, vatype);                                   \
    va_end(va);                                                         \
    return array->cnt;                                                  \
  }                                                                     \
  size_t type##_write(ctype* array, size_t n, eltype* elements) {       \
    assert(elements != NULL);                                           \
    size_t oldN = array->cnt;                                           \
    resize_##type(array, array->cnt+n);                                 \
    eltype* space = array->data + oldN;                                 \
    memcpy(space, elements, n * sizeof(eltype));                        \
    return array->cnt;                                                  \
  }                                                                     \
  eltype type##_pop(ctype* array) {                                     \
    rl_require(array->cnt > 0,                                          \
               NOTHING,                                                 \
               NULL,                                                    \
               "<runtime @ "#type"_pop>",                               \
               "underflow");                                            \
    eltype out = array->data[array->cnt-1];                             \
    resize_##type(array, array->cnt-1);                                 \
    return out;                                                         \
  }                                                                     \
  eltype type##_popn(ctype* array, size_t n) {                          \
    rl_require(array->cnt >= n,                                         \
               NOTHING,                                                 \
               NULL,                                                    \
               "<runtime @ "#type"_popn>",                              \
               "underflow" );                                           \
    if ( n == 0 )                                                       \
      return (eltype)0;                                                 \
    eltype out = array->data[array->cnt-n];                             \
    resize_##type(array, array->cnt-n);                                 \
    return out;                                                         \
  }

// global variables +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
vm_t Vm = {
  .reader={
    .subexpressions={
      .data=NULL,
      .cnt =0,
      .cap =0
    },

    .buffer={
      .data=NULL,
      .cnt =0,
      .cap =0
    }
  },

  .heap={
    .grays={
      .data=NULL,
      .cnt =0,
      .cap =0
    },

    .live=NULL,
    .used=0,
    .max =HEAPSIZE
  },

  .symbol_table={
    .root=NULL
  }
};

list_t EmptyList = {
  .obj={ NULL, LIST, true, false },
  .arity=0,
  .head=NUL,
  .tail=&EmptyList
};

// special globals ------------------------------------------------------------


// special constants ----------------------------------------------------------
value_t True, False;

// special forms --------------------------------------------------------------
value_t Quote, Do, If, Def, Put;

// native functions -----------------------------------------------------------
value_t Add, Sub, Mul, Div, Eql, Not;
value_t Head, Tail, Cons, Len;
value_t NumberP, NativeP, SymbolP, ListP, ConsP, EmptyP, NulP;

// implementations ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// array implementations ------------------------------------------------------
ARRAY_API(objects, objects_t, object_t*);
ARRAY_IMPL(objects, objects_t, object_t*, object_t*, false);
ARRAY_API(values, values_t, value_t);
ARRAY_IMPL(values, values_t, value_t, value_t, false);
ARRAY_API(buffer, buffer_t, char);
ARRAY_IMPL(buffer, buffer_t, char, int, true);

// runtime implementations ----------------------------------------------------
void mark_object(void* ptr);
void mark_value(value_t val);

#define mark(x) _Generic((x), value_t:mark_value, default:mark_object)(x)

void trace_object(void* ptr);
void trace_value(value_t val);

#define trace(x) _Generic((x), value_t:trace_value, default:trace_object)(x)

void finalize_object(void* ptr);
void finalize_value(value_t val);

#define finalize(x) _Generic((x), value_t:finalize_value, default:finalize_object)(x)

void gc_mark_phase(void);
void gc_trace_phase(void);
void gc_sweep_phase(void);
void gc_cleanup_phase(void);

// mark dispatch & helpers ----------------------------------------------------
void mark_object(void* ptr) {
  if ( ptr == NULL )
    return;

  object_t* obj = ptr;

  if ( obj->black )
    return;

  obj->black = true;
  objects_push(&Vm.heap.grays, obj);
}

void mark_value(value_t val) {
  if ( is_object(val) )
    mark_object(as_pointer(val));
}

void mark_values(values_t* values) {
  for (size_t i=0; i<values->cnt; i++)
    mark(values->data[i]);
}

// trace dispatch & helpers ---------------------------------------------------
static void trace_list(list_t* xs) {
  mark(xs->head);
  mark(xs->tail);
}

static void trace_symbol(symbol_t* sym) {
  if (sym == NULL)
    return;

  mark(sym->bind);
  mark(sym->left);
  mark(sym->right);
}

static void trace_native(native_t* native) {
  mark(native->name);
}

static void trace_environment(environment_t* environment) {
  mark(environment->names);
  mark(environment->binds);
  mark(environment->parent);
}

void trace_object(void* ptr) {
  if ( ptr == NULL )
    return;

  object_t* obj = ptr;
  obj->gray     = false;

  switch ( obj->type ) {
    case NATIVE:      trace_native(ptr);      break;
    case ENVIRONMENT: trace_environment(ptr); break;
    case SYMBOL:      trace_symbol(ptr);      break;
    case LIST:        trace_list(ptr);        break;
    default:                                  break;
  }
}

void trace_value(value_t val) {
  if ( is_object(val) )
    trace_object(as_pointer(val));
}

// finalize dispatch & helpers ------------------------------------------------
static void finalize_symbol(symbol_t* sym) {
  deallocate(sym->name, strlen(sym->name)+1, false);
}

void finalize_object(void* ptr) {
  assert(ptr != NULL);

  object_t* obj = ptr;

  switch ( obj->type ) {
    case SYMBOL: finalize_symbol(ptr); break;
    default:                           break;
  }

  deallocate(ptr, type_size(obj->type), true);
}

void finalize_value(value_t val) {
  if ( is_object(val) )
    finalize_object(as_pointer(val));
}

// misc memory helpers --------------------------------------------------------
#define HLF 0.625

bool check_heap_overflow(size_t n) {
  return n + Vm.heap.used >= Vm.heap.max;
}

bool check_heap_resize(void) {
  return (double)Vm.heap.used / (double)Vm.heap.max >= HLF;
}

void resize_heap(void) {
  Vm.heap.max <<= 1;
}

// gc phases ------------------------------------------------------------------
void gc_mark_phase(void) {
  // mark gc roots ------------------------------------------------------------
  mark(Vm.symbol_table.root);
  mark(&EmptyList);
  mark_values(&Vm.reader.subexpressions);
}

void gc_trace_phase(void) {
  // trace grays --------------------------------------------------------------
  while ( Vm.heap.grays.cnt ) {
    object_t* obj = objects_pop(&Vm.heap.grays);
    trace(obj);
  }
}

void gc_sweep_phase(void) {
  object_t *curr = Vm.heap.live, **prev = &Vm.heap.live;

  while ( curr != NULL ) {
    if ( curr->black ) {
      curr->black = false;
      curr->gray  = true;
      prev        = &curr->next;
      curr        = curr->next;
    } else {
      object_t* tmp = curr;
      *prev         = curr = tmp->next;
      finalize(tmp);
    }
  }
}

void gc_cleanup_phase(void) {
  // grow heap (if necessary) -------------------------------------------------
  if ( check_heap_resize() )
    resize_heap();
}

void manage(void) {
  gc_mark_phase();
  gc_trace_phase();
  gc_sweep_phase();
  gc_cleanup_phase();
}

void* allocate(size_t n, bool fromHeap) {
  if ( fromHeap ) {
    if ( check_heap_overflow(n) )
      manage();

    Vm.heap.used += n;
  }

  void* out = malloc(n);

  if (out == NULL) {
    fprintf(stderr, "<runtime @ allocate>:error: out of memory.\n");
    exit(1);
  }

  memset(out, 0, n);

  return out;
}

void* allocate_array(size_t n, size_t o, bool fromHeap) {
  return allocate(n * o, fromHeap);
}

void* duplicate(void* ptr, size_t n, bool fromHeap) {
  void* out = allocate(n, fromHeap);
  memcpy(out, ptr, n);
  return out;
}

void* duplicate_array(void* ptr, size_t n, size_t o, bool fromHeap) {
  return duplicate(ptr, n*o, fromHeap);
}

char* duplicate_string(const char* s, bool fromHeap) {
  size_t l = strlen(s);
  return duplicate_array((void*)s, l+1, sizeof(char), fromHeap);
}

void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap) {
  void* out = NULL;

  if (newN == 0) {
    if (oldN > 0)
      deallocate(ptr, oldN, fromHeap);
    
  } else if ( newN > oldN ) {
    size_t diff = newN - oldN;
    
    if ( fromHeap ) {
      if ( check_heap_overflow(diff) )
        manage();
      
      Vm.heap.used += diff;      
    }
    
    out = realloc(ptr, newN);
    
    if ( out == NULL ) {
      fprintf(stderr, "<runtime @ allocate>:error: out of memory.\n");
      exit(1);
    }
    
    memset(out+oldN, 0, diff);
  } else {
    size_t diff   = oldN - newN;
    
    if ( fromHeap )
      Vm.heap.used -= diff;
    
    out           = realloc(ptr, newN);
    
    if ( out == NULL ) {
      fprintf(stderr, "<runtime @ allocate>: error: out of memory.\n");
      exit(1);
    }
  }

  return out;
}

void* reallocate_array(void* ptr, size_t oldN, size_t newN, size_t o, bool fromHeap) {
  return reallocate(ptr, oldN*o, newN*o, fromHeap);
}

void  deallocate(void* ptr, size_t n, bool fromHeap) {
  if ( fromHeap ) {
    assert(n <= Vm.heap.used);
    Vm.heap.used -= n;
  }

  if (ptr)
    free(ptr);
}

void  deallocate_array(void* ptr, size_t n, size_t o, bool fromHeap) {
  return deallocate(ptr, n*o, fromHeap);
}

// stack API ------------------------------------------------------------------
void    init_interpreter(void) {
  init_values(&Vm.interpreter.stack);
}

void    free_interpreter(void) {
  free_values(&Vm.interpreter.stack);
}

void    reset_interpreter(void) {
  reset_values(&Vm.interpreter.stack);
}

size_t  push(value_t val) {
  return values_push(&Vm.interpreter.stack, val);
}

size_t pushn(size_t n, ...) {
  size_t offset = Vm.interpreter.stack.cnt;
  resize_values(&Vm.interpreter.stack, Vm.interpreter.stack.cnt+n);
  
  value_t* buffer = Vm.interpreter.stack.data+offset;
  va_list va;
  va_start(va, n);

  for ( size_t i=0; i<n; i++ )
    buffer[i] = va_arg(va, value_t);

  va_end(va);
  
  return offset;
}

value_t pop(void) {
  return values_pop(&Vm.interpreter.stack);
}

value_t popn(size_t n) {
  return values_popn(&Vm.interpreter.stack, n);
}

value_t* peek(size_t n) {
  assert(n < Vm.interpreter.stack.cnt);
  return Vm.interpreter.stack.data + n;
}

// type implementations -------------------------------------------------------
// object 'type' --------------------------------------------------------------
SAFECAST(object_t*, object, as_pointer);

bool  is_object(value_t x) {
  return (x & TAGM) == OBJ;
}

void  init_object(void* ptr, type_t type) {
  object_t* obj = ptr;
  obj->next     = Vm.heap.live;
  obj->type     = type;
  obj->gray     = true;
  obj->black    = false;

  Vm.heap.live  = obj;
}

void* mk_object(type_t type) {
  object_t* out = allocate(type_size(type), true);

  init_object(out, type);

  return out;
}

// number type ----------------------------------------------------------------
SAFECAST(number_t, number, as_number);
TYPEP(number, NUMBER);

value_t mk_number(number_t number) {
  return ((ieee754_64_t)number).word;
}

// native type ----------------------------------------------------------------
SAFECAST(native_t*, native, as_pointer);
TYPEP(native, NATIVE);

value_t  mk_native(symbol_t* name, bool special, bool variadic, size_t arity, native_callback_t callback) {
  native_t* native = mk_object(NATIVE);
  native->name     = name;
  native->special  = special;
  native->variadic = variadic;
  native->arity    = arity;
  native->callback = callback;

  return tag_pointer(native, OBJ);
}

value_t def_native(const char* fname, bool special, bool variadic, size_t arity, native_callback_t callback) {
  symbol_t* sym  = intern_symbol(fname);
  value_t native = mk_native(sym, special, variadic, arity, callback);
  sym->bind      = native;

  return tag_pointer(sym, OBJ);
}

// unit type ------------------------------------------------------------------
TYPEP(unit, UNIT);

// symbol type ----------------------------------------------------------------
SAFECAST(symbol_t*, symbol, as_pointer);
TYPEP(symbol, SYMBOL);

static symbol_t** find_symbol(symbol_t** root, const char* token) {
  while ( *root ) {
    int o = strcmp(token, (*root)->name);

    if ( o < 0 )
      root = &(*root)->left;

    else if ( o > 0 )
      root = &(*root)->right;

    else
      break;
  }

  return root;
}

static symbol_t* new_symbol(const char* token) {
  symbol_t* out = mk_object(SYMBOL);
  out->left     = NULL;
  out->right    = NULL;
  out->name     = duplicate_string(token, false);
  out->bind     = NUL;

  return out;
}

symbol_t* intern_symbol(const char* token) {
  symbol_t** location = find_symbol(&Vm.symbol_table.root, token);

  if (*location == NULL)
    *location = new_symbol(token);

  return *location;
}

value_t mk_symbol(const char* token) {
  symbol_t* out = intern_symbol(token);

  return tag_pointer(out, OBJ);
}

value_t def_toplevel(const char* name, value_t value) {
  symbol_t* sym = intern_symbol(name);

  if ( value == NOTHING )          // create a self-evaluating symbol
    value = tag_pointer(sym, OBJ);

  sym->bind = value;
  return tag_pointer(sym, OBJ);
}

// list type ------------------------------------------------------------------
SAFECAST(list_t*, list, as_pointer);
TYPEP(list, LIST);

static void init_list(list_t* slf, value_t head, list_t* tail) {
  init_object(slf, LIST);
  slf->arity = tail->arity + 1;
  slf->head  = head;
  slf->tail  = tail;
}

value_t mk_list(value_t head, list_t* tail) {
  pushn(2, head, tag_pointer(tail, OBJ));
  list_t* out = mk_object(LIST);
  popn(2);
  out->arity  = tail->arity+1;
  out->head   = head;
  out->tail   = tail;

  return tag_pointer(out, OBJ);
}

value_t mk_list2(value_t fst, value_t snd) {
  size_t offset = pushn(2, fst, snd);
  value_t out   = mk_listn(2, peek(offset));
  popn(2);
  return out;
}

value_t mk_listn(size_t n, value_t* args) {
  list_t* out;

  if ( n == 0 )
    out = &EmptyList;

  else {
    out = allocate_array(n, sizeof(list_t), true);
    list_t* curr = out + n - 1, * prev = &EmptyList;

    for ( size_t i=n; i > 0; i--, prev=curr, curr-- )
      init_list(curr, args[i-1], prev);
  }

  return tag_pointer(out, OBJ);
}

value_t list_nth(const char* fname, list_t* xs, size_t n) {
  rl_require(n > 0 && n <= xs->arity,
             NOTHING,
             NULL,
             fname,
             "%zu out of bounds for list of arity %zu",
             n,
             xs->arity );

  while ( --n )
    xs = xs->tail;

  return xs->head;
}

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v) {
  switch (v & TAGM) {
    case NUL:  return UNIT;
    case OBJ:  return as_object(v)->type;
    default:   return NUMBER;
  }
}

// interpreter functions ------------------------------------------------------
// error & error helpers ------------------------------------------------------
static void print_error(value_t cause, list_t* form, const char* fname, const char* fmt, va_list va) {
  fprintf(stderr, "\n%s: error: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");

  if ( cause != NOTHING ) {
    fprintf(stderr, "caused by: " );
    rl_println(cause);
  }

  if ( form != NULL ) {
    fprintf(stderr, "in expression: ");
    rl_println(tag_pointer(form, OBJ));
  }   
}

void rl_error(value_t cause, list_t* form, const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  print_error(cause, form, fname, fmt, va);
  va_end(va);
  longjmp(Vm.error.jmpbuf, 1);
}

void argcount(list_t* form, const char* fname, bool variadic, size_t expect, size_t got) {
  if ( variadic )
    rl_require(got >= expect,
               NOTHING,
               form,
               fname,
               "expected at least %zu inputs to #, got %zu",
               expect,
               got );

  else
    rl_require(got == expect,
               NOTHING,
               form,
               fname,
               "expected %zu inputs to #, got %zu",
               expect,
               got );
}

void argtype(value_t got, list_t* form, const char* fname, type_t expect) {
  rl_require(rl_type_of(got) == expect,
             got,
             form,
             fname,
             "expected a value of type %s",
             type_name(expect) );
}

// reader helpers -------------------------------------------------------------
static value_t read_expression(FILE* stream);

void init_reader(bool total);
void free_reader(bool total);
void reset_reader(bool total);

void init_reader(bool total) {
  if ( total )
    init_values(&Vm.reader.subexpressions);

  init_buffer(&Vm.reader.buffer);
}

void free_reader(bool total) {
  if ( total )
    free_values(&Vm.reader.subexpressions);

  free_buffer(&Vm.reader.buffer);
}

void reset_reader(bool total) {
  free_reader(total);
  init_reader(total);
}

static int fpeekc(FILE* stream) {
  int ch = fgetc(stream);

  if (ch != EOF)
    ungetc(ch, stream);

  return ch;
}

static int peekc(void) {
  return fpeekc(stdin);
}

static bool isrlws(int ch) {
  return ch == ',' || isspace(ch);
}

static bool isrlterm(int ch) {
  return ch == EOF || isspace(ch) || ch == ',' || ch == ')';
}

static int skiprlws(FILE* stream) {
  int out = fpeekc(stream);

  while ( out != EOF && isrlws(out) ) {
    fgetc(stream);
    out = fpeekc(stream);
  }

  return out;
}

static size_t accumc(int ch) {
  return buffer_push(&Vm.reader.buffer, ch);
}

static value_t read_atom(int dispatch, FILE* stream) {
  value_t out;

  while ( !isrlterm(dispatch) ) {
    accumc(dispatch);
    fgetc(stream);
    dispatch = fpeekc(stream);
  }

  char* token = Vm.reader.buffer.data;

  if ( strcmp(token, "nul") == 0 )
    out = NUL;

  else {
    char* num_end;
    number_t num = strtod(token, &num_end);

    if ( *num_end == '\0' )
      out = mk_number(num);

    else
      out = mk_symbol(token);
  }

  return out;
}

static value_t read_list(int dispatch, FILE* stream) {
  fgetc(stream); // clear opening '('
  size_t base = Vm.reader.subexpressions.cnt; // index of first value

  dispatch = fpeekc(stream);

  while ( dispatch != ')' && dispatch != EOF ) {
    value_t x = read_expression(stream);
    values_push(&Vm.reader.subexpressions, x);
    dispatch = fpeekc(stream);
  }

  rl_require(dispatch != EOF,
             NOTHING,
             NULL,
             "read",
             "unexpected EOF reading list");
  fgetc(stream); // clear closing ')'

  size_t n    = Vm.reader.subexpressions.cnt - base;
  value_t* a  = Vm.reader.subexpressions.data + base;
  value_t out = mk_listn(n, a);

  values_popn(&Vm.reader.subexpressions, n); // remove saved subexpressions

  return out;
}

static value_t read_quote(int dispatch, FILE* stream) {
  fgetc(stream);               // clear '
  dispatch = skiprlws(stream);
  
  rl_require(dispatch != EOF,  // require a quoted expression
             NOTHING,
             NULL,
             "read",
             "unexpected EOF reading quote");

  value_t quoted = read_expression(stream);
  return mk_list2(Quote, quoted);
}

static value_t read_expression(FILE* stream) {
  reset_reader(false); // clear last token
  value_t out = NUL;
  int ch = skiprlws(stream);

  switch (ch) {
    case ')':  rl_error(NOTHING, NULL, "read", "Unmatched ')'"); break;

    case '\'': out = read_quote(ch, stream); break;
    case '(':  out = read_list(ch, stream);  break;
    default:   out = read_atom(ch, stream);  break;
  }

  reset_reader(false); // clear last token
  return out;
}

value_t rl_read(void) {
  reset_reader(true);
  value_t out = read_expression(stdin);
  reset_reader(true);
  return out;
}

// eval implementation --------------------------------------------------------
list_t* eval_args(list_t* args, environment_t* environment) {
  size_t offset = push(tag_pointer(args, OBJ));
  size_t n = 0;

  for ( ; args->arity; n++, args=args->tail ) {
    value_t v = rl_eval(args->head, environment);
    push(v);
  }

  value_t out = mk_listn(n, peek(offset));
  popn(n+1);
  return as_pointer(out);
}

static value_t eval_symbol(value_t v, environment_t* environment) {
  while ( environment != NULL ) {
    
  }

  return to_symbol(NULL, "eval", v)->bind;
}

static value_t eval_list(value_t v, environment_t* environment) {
  if ( v == tag_pointer(&EmptyList, OBJ) ) // treat empty list as a literal
    return v;
  
  push(v);                             // save in case of GC
  list_t* form   = to_list(NULL, "eval", v);
  value_t head   = form->head;
  list_t* args   = form->tail;
  head           = rl_eval(head, environment);

  argtype(head, form, "eval", NATIVE);
  native_t* native = as_pointer(head);
  argcount(form, native->name->name, native->variadic, native->arity, args->arity);
  value_t out;

  if (!native->special)
    args = eval_args(args, environment);

  out = native->callback(form, environment, args);

  pop();                               // remove form from stack

  return out;
}

static value_t eval_literal(value_t v, environment_t* environment) {
  (void)environment;
  return v;
}

value_t rl_eval(value_t v, environment_t* environment) {
  if ( is_symbol(v) )
    return eval_symbol(v, environment);

  else if ( is_list(v) )
    return eval_list(v, environment);

  else
    return eval_literal(v, environment);
}

// print implementations ------------------------------------------------------
static void print_number(value_t val, bool newline) {
  if ( newline )
    printf("%g\n", to_number(NULL, "print", val));

  else
    printf("%g", to_number(NULL, "print", val));    
}

static void print_unit(value_t val, bool newline) {
  (void)val;

  if ( newline )
    printf("nul\n");

  else
    printf("nul");
}

static void print_symbol(value_t val, bool newline) {
  if ( newline )
    printf("%s\n", to_symbol(NULL, "print", val)->name);

  else
    printf("%s", to_symbol(NULL, "print", val)->name);    
}

static void print_list(value_t val, bool newline) {
  printf("(");

  list_t* xs = to_list(NULL, "print", val);

  for ( ; xs->arity > 0; xs=xs->tail ) {
    rl_print(xs->head);

    if ( xs->arity > 1 )
      printf(" ");
  }

  printf(newline ? ")\n" : ")");
}

static void print_native(value_t val, bool newline) {
  const char* fmt = newline ? "<native function %s>\n" : "<native function %s>";
  printf(fmt, to_native(NULL, "print", val)->name->name);
}

static void print_value(value_t val, bool newline) {
  const char* fmt = newline ? "<%s @ %lx>\n" : "<%s @ %lx>";  
  printf(fmt, type_name(rl_type_of(val)), val);
}

void rl_print(value_t v) {
  type_t type = rl_type_of(v);

  switch (type) {
    case NUMBER: print_number(v, false); break;
    case UNIT:   print_unit(v, false); break;
    case NATIVE: print_native(v, false); break;
    case SYMBOL: print_symbol(v, false); break;
    case LIST:   print_list(v, false); break;
    default:     print_value(v, false); break;
  }
}

void rl_println(value_t v) {
  type_t type = rl_type_of(v);

  switch (type) {
    case NUMBER: print_number(v, true); break;
    case UNIT:   print_unit(v, true); break;
    case NATIVE: print_native(v, true); break;
    case SYMBOL: print_symbol(v, true); break;
    case LIST:   print_list(v, true); break;
    default:     print_value(v, true); break;
  }
}

// native functions -----------------------------------------------------------
// helpers --------------------------------------------------------------------
static value_t rl_bool(bool test) {
  return test ? True : NUL;
}
// special forms --------------------------------------------------------------
value_t native_quote(list_t* form, environment_t* environment, list_t* args) {
  (void)form;

  return args->head;
}

value_t native_do(list_t* form, environment_t* environment, list_t* args) {
  (void)form;

  for ( ; args->arity > 1; args=args->tail )
    rl_eval(args->head, environment); // initial subexpressions evaluated for side effects only

  return rl_eval(args->head, environment);
}

value_t native_if(list_t* form, environment_t* environment, list_t* args) {
  rl_require(args->arity == 2 || args->arity == 3,
             NOTHING,
             form,
             "if",
             "expected 2 or 3 inputs, got %zu",
             args->arity);

  value_t test = args->head;
  value_t csqt = args->tail->head;
  value_t alt  = args->tail->tail->head;
  value_t x    = rl_eval(test, environment);
  value_t v    = x == NUL ? rl_eval(alt, environment) : rl_eval(csqt, environment);

  return v;
}

// arithmetic -----------------------------------------------------------------
value_t native_add(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  number_t out = 0;

  for ( ; args->arity; args=args->tail )
    out += to_number(form, "+", args->head);

  return mk_number(out);
}

value_t native_sub(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  if ( args->arity == 1 )
    return mk_number(-to_number(form, "-", args->head));

  number_t out = to_number(form, "-", args->head);
  args         = args->tail;

  for ( ; args->arity; args=args->tail )
    out -= to_number(form, "-", args->head);

  return mk_number(out);
}

value_t native_mul(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  number_t out = to_number(form, "*", args->head);
  args         = args->tail;

  for ( ; args->arity && out != 0; args=args->tail )
    out *= to_number(form, "*", args->head);

  return mk_number(out);
}

value_t native_div(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  number_t out = to_number(form, "/", args->head);

  if ( args->arity == 1 ) {
    rl_require(out != 0, NOTHING, form, "/", "zero-division");
    out = 1 / out;
  }

  else {
    args = args->tail;

    for ( ; out != 0 && args->arity; args=args->tail ) {
      number_t denom = to_number(form, "/", args->head);
      rl_require(denom != 0, NOTHING, form, "/", "zero-division");
      out /= denom;
    }
  }

  return mk_number(out);
}

value_t native_eql(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  number_t first = to_number(form, "=", args->head);
  value_t out = True;
  args = args->tail;

  for ( ; out != NUL && args->arity; args=args->tail ) {
    number_t second = to_number(form, "=", args->head);

    if ( first != second )
      out = NUL;
  }

  return out;
}

value_t native_not(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  (void)form;
  return args->head == NUL ? True : NUL;
}

value_t native_head(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  list_t* xs = to_list(form, "head", args->head);
  rl_require(xs->arity > 0,
             NOTHING,
             form,
             "head",
             "head called on empty list");

  return xs->head;
}

value_t native_tail(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  list_t* xs = to_list(form, "tail", args->head);
  rl_require(xs->arity > 0,
             NOTHING,
             form,
             "tail",
             "tail called on empty list");

  return tag_pointer(xs->tail, OBJ);
}

value_t native_cons(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  value_t head = args->head;
  list_t* tail = to_list(form, "cons", args->tail->head);
  return mk_list(head, tail);
}

value_t native_len(list_t* form, environment_t* environment, list_t* args) {
  (void)environment;
  list_t* xs = to_list(form, "len", args->head);
  return mk_number(xs->arity);
}

#define NATIVE_TYPEP(type)                                              \
  value_t native_##type##p(list_t* form, environment_t* environment, list_t* args) {     \
    (void)environment;                                                  \
    (void)form;                                                         \
    return is_##type(args->head) ? True : NUL;                          \
  }

NATIVE_TYPEP(number);
NATIVE_TYPEP(native);
NATIVE_TYPEP(environment);
NATIVE_TYPEP(symbol);
NATIVE_TYPEP(list);

value_t native_consp(list_t* form, environment_t* environment, list_t* args) {
  (void)form;
  (void)environment;
  value_t x = args->head;
  return rl_bool(is_list(x) && to_list(form, "cons?", x)->arity > 0);
}

value_t native_emptyp(list_t* form, environment_t* environment, list_t* args) {
  (void)form;
  (void)environment;
  value_t x = args->head;
  return rl_bool(is_list(x) && to_list(form, "empty?", x)->arity == 0);
}

value_t native_nulp(list_t* form, environment_t* environment, list_t* args) {
  (void)form;
  (void)environment;
  return rl_bool(args->head == NUL);
}

// repl implementation --------------------------------------------------------
#define PROMPT  "rascal>"

void rl_repl(void) {
  while (true) {
    if (savepoint()) {
      reset_interpreter();
      reset_reader(true);
      newline();
      continue;
    }

    printf(PROMPT" ");
    value_t x = rl_read();
    value_t v = rl_eval(x, NULL);
    rl_print(v);
    newline();
 }
}

// main +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// version information --------------------------------------------------------
#define VERSION "%d.%d.%d.%c"
#define MAJOR 0
#define MINOR 0
#define PATCH 9
#define DEVELOPMENT 'a'

// prompts/messages -----------------------------------------------------------
#define WELCOME "Welcome to Rascal version "VERSION"!"
#define GOODBYE "Later blood!"

// startup helpers ------------------------------------------------------------
static void initialize_rascal(void) {
  init_interpreter();
  init_reader(true);

  // initialize special forms
  Quote = def_native("quote", true, false, 1, native_quote);
  Do    = def_native("do", true, true, 1, native_do);
  If    = def_native("if", true, true, 2, native_if);
  Def   = def_native("def", true, false, 2, native_def);
  Put   = def_native("put", true, false, 2, native_put);

  // initialize native functions
  Add     = def_native("+", false, true, 1, native_add);
  Sub     = def_native("-", false, true, 1, native_sub);
  Mul     = def_native("*", false, true, 1, native_mul);
  Div     = def_native("/", false, true, 1, native_div);
  Eql     = def_native("=", false, true, 2, native_eql);
  Not     = def_native("not", false, false, 1, native_not);
  Head    = def_native("head", false, false, 1, native_head);
  Tail    = def_native("tail", false, false, 1, native_tail);
  Cons    = def_native("cons", false, false, 2, native_cons);
  Len     = def_native("len", false, false, 1, native_len);
  NumberP = def_native("number?", false, false, 1, native_numberp);
  NativeP = def_native("native?", false, false, 1, native_nativep);
  SymbolP = def_native("symbol?", false, false, 1, native_symbolp);
  ListP   = def_native("list?", false, false, 1, native_listp);
  ConsP   = def_native("cons?", false, false, 1, native_consp);
  EmptyP  = def_native("empty?", false, false, 1, native_emptyp);
  NulP    = def_native("nul?", false, false, 1, native_nulp);

  // initialize special constants
  True  = def_toplevel("true", NOTHING);
  False = def_toplevel("false", NUL);
}

static void finalize_rascal(void) {
  free_interpreter();
  free_reader(true);
}

static void run_rascal(const int argc, const char* argv[]) {
  (void)argc;
  (void)argv;
  rl_repl();
}

static void welcome_message(void) {
  printf(WELCOME"\n", MAJOR, MINOR, PATCH, DEVELOPMENT);
}

static void goodbye_message(void) {
  printf(GOODBYE"\n");
}

// main -----------------------------------------------------------------------
int main( const int argc, const char* argv[] ) {
  initialize_rascal();
  welcome_message();
  run_rascal(argc, argv);
  goodbye_message();
  finalize_rascal();
  return 0;
}
