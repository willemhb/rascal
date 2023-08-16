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
  SYMBOL,
  LIST
} type_t;

// object types (forward) -----------------------------------------------------
typedef struct object object_t;
typedef struct symbol symbol_t;
typedef struct list list_t;

// value types ----------------------------------------------------------------
typedef uintptr_t value_t;
typedef double number_t;
typedef value_t (*native_t)(list_t* args);

struct object {
  object_t* next;
  type_t    type;
  short     gray;
  short     black;
};

// object types ---------------------------------------------------------------
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
#define NTV  0x7ffd000000000000UL
#define OBJ  0x7ffe000000000000UL

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
char* duplicate_string(char* string, bool fromHeap);
void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap);
void* reallocate_array(void* ptr, size_t oldN, size_t newN, size_t o, bool fromHeap);
void  deallocate(void* ptr, size_t n, bool fromHeap);
void  deallocate_array(void* ptr, size_t n, size_t o, bool fromHeap);

// type implementations -------------------------------------------------------
// number type ----------------------------------------------------------------
number_t to_number(const char* fname, value_t value);
value_t  mk_number(number_t number);
bool     is_number(value_t x);

// native type ----------------------------------------------------------------
native_t to_native(const char* fname, value_t value);
value_t  mk_native(native_t native);
bool     is_native(value_t x);

// unit type ------------------------------------------------------------------
bool is_unit(value_t x);

// symbol type ----------------------------------------------------------------
symbol_t* to_symbol(const char* fname, value_t value);
value_t   mk_symbol(char* token);
bool      is_symbol(value_t x);

// list type ------------------------------------------------------------------
list_t* to_list(const char* fname, value_t value);
value_t mk_list(value_t head, list_t* tail);
bool    is_list(value_t x);
value_t mk_listn(size_t n, value_t* args);

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v);

// interpreter functions ------------------------------------------------------
void    rl_error(const char* fname, const char* fmt, ...);
value_t rl_read(void);
value_t rl_eval(value_t v);
void    rl_print(value_t v);
void    rl_repl(void);

// utility macros & statics +++++++++++++++++++++++++++++++++++++++++++++++++++
#define rl_require(test, fname, message, ...)               \
  do {                                                      \
    if (!(test))                                            \
      rl_error(fname, message __VA_OPT__(,) __VA_ARGS__);   \
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
    case NUMBER: out = "number";  break;
    case UNIT:   out = "unit";    break;
    case SYMBOL: out = "symbol";  break;
    default:     out = "unknown"; break;
  }

  return out;
}

static size_t type_size(type_t type) {
  size_t out;
  
  switch ( type ) {
    case NUMBER: out = sizeof(number_t); break;
    case UNIT:   out = sizeof(value_t);  break;
    case SYMBOL: out = sizeof(symbol_t); break;
    case LIST:   out = sizeof(list_t);   break;
    default:     out = 0; break;
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
#define SAFECAST(ctype, type, convert)                           \
  ctype to_##type(const char* fname, value_t val) {              \
    rl_require(is_##type(val), fname, "not a %s", #type);        \
    return (ctype)convert(val);                                  \
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
  size_t type##_write(ctype* array, size_t n, eltype* elements);       \
  eltype type##_pop(ctype* array);                                     \
  eltype type##_popn(ctype* array, size_t n)

#define ARRAY_IMPL(type, ctype, eltype, encoded)                        \
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
    rl_require(n <= MAXSIZE, "<runtime @ resize_"#type">", "overflow"); \
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
  size_t type##_write(ctype* array, size_t n, eltype* elements) {       \
    assert(elements != NULL);                                           \
    size_t oldN = array->cnt;                                           \
    resize_##type(array, array->cnt+n);                                 \
    eltype* space = array->data + oldN;                                 \
    memcpy(space, elements, n * sizeof(eltype));                        \
    return array->cnt;                                                  \
  }                                                                     \
  eltype type##_pop(ctype* array) {                                     \
    rl_require(array->cnt > 0, "<runtime @ "#type"_pop>", "underflow"); \
    eltype out = array->data[array->cnt-1];                             \
    resize_##type(array, array->cnt-1);                                 \
    return out;                                                         \
  }                                                                     \
  eltype type##_popn(ctype* array, size_t n) {                          \
    rl_require(array->cnt >= n, "<runtime @ "#type"_popn>", "underflow"); \
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

// implementations ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// array implementations ------------------------------------------------------
ARRAY_API(objects, objects_t, object_t*);
ARRAY_IMPL(objects, objects_t, object_t*, false);
ARRAY_API(values, values_t, value_t);
ARRAY_IMPL(values, values_t, value_t, false);
ARRAY_API(buffer, buffer_t, char);
ARRAY_IMPL(buffer, buffer_t, char, true);

// runtime implementations ----------------------------------------------------
void* allocate(size_t n, bool fromHeap) {
  (void)fromHeap;

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

char* duplicate_string(char* s, bool fromHeap) {
  size_t l = strlen(s);
  return duplicate_array(s, l+1, sizeof(char), fromHeap);
}

void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap) {
  if (newN == 0) {
    deallocate(ptr, oldN, fromHeap);
    return NULL;
  }

  ptr = realloc(ptr, newN);

  if (ptr == NULL) {
    fprintf(stderr, "<runtime @ allocate>:error: out of memory.\n");
    exit(1);
  }

  if (newN > oldN)
    memset(ptr+oldN, 0, newN-oldN);

  return ptr;
}

void* reallocate_array(void* ptr, size_t oldN, size_t newN, size_t o, bool fromHeap) {
  return reallocate(ptr, oldN*o, newN*o, fromHeap);
}

void  deallocate(void* ptr, size_t n, bool fromHeap) {
  (void)fromHeap;
  (void)n;

  if (ptr)
    free(ptr);
}

void  deallocate_array(void* ptr, size_t n, size_t o, bool fromHeap) {
  return deallocate(ptr, n*o, fromHeap);
}


// type implementations -------------------------------------------------------
// object type ----------------------------------------------------------------
void  init_object(void* ptr, type_t type) {
  object_t* obj = ptr;

  obj->type     = type;
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

// unit type ------------------------------------------------------------------
TYPEP(unit, UNIT);

// symbol type ----------------------------------------------------------------
SAFECAST(symbol_t*, symbol, as_pointer);
TYPEP(symbol, SYMBOL);

static symbol_t** find_symbol(symbol_t** root, char* token) {
  while (*root) {
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

static symbol_t* new_symbol(char* token) {
  symbol_t* out = mk_object(SYMBOL);
  out->left     = NULL;
  out->right    = NULL;
  out->name     = duplicate_string(token, false);
  out->bind     = NUL;

  return out;
}

value_t mk_symbol(char* token) {
  symbol_t** location = find_symbol(&Vm.symbol_table.root, token);

  if (*location == NULL)
    *location = new_symbol(token);

  return tag_pointer(*location, OBJ);
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
  list_t* out = mk_object(LIST);
  out->arity  = tail->arity+1;
  out->head   = head;
  out->tail   = tail;

  return tag_pointer(out, OBJ);
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

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v) {
  switch (v & TAGM) {
    case NUL:  return UNIT;
    case NTV:  return NATIVE;
    case OBJ:  return as_object(v)->type;
    default:   return NUMBER;
  }
}

// interpreter functions ------------------------------------------------------
void rl_error(const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "\n%s: error: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);
  longjmp(Vm.error.jmpbuf, 1);
}

// reader helpers -------------------------------------------------------------
static value_t read_expression(FILE* stream);
static void    reset_reader(bool total);

static void init_reader(bool total) {
  if ( total )
    init_values(&Vm.reader.subexpressions);

  init_buffer(&Vm.reader.buffer);
}

static void free_reader(bool total) {
  if ( total )
    free_values(&Vm.reader.subexpressions);

  free_buffer(&Vm.reader.buffer);
}

static void reset_reader(bool total) {
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

  rl_require(dispatch != EOF, "read", "unexpected EOF reading list");
  fgetc(stream); // clear closing ')'

  size_t n    = Vm.reader.subexpressions.cnt - base;
  value_t* a  = Vm.reader.subexpressions.data + base;
  value_t out = mk_listn(n, a);

  values_popn(&Vm.reader.subexpressions, n); // remove saved subexpressions

  return out;
}

static value_t read_expression(FILE* stream) {
  reset_reader(false); // clear last token
  value_t out = NUL;
  int ch = skiprlws(stream);

  switch (ch) {
    case ')': rl_error("read", "Unmatched ')'"); break;

    case '(': out = read_list(ch, stream); break;
    default:  out = read_atom(ch, stream); break;
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
static value_t eval_literal(value_t v) {
  return v;
}

static value_t eval_symbol(value_t v) {
  return to_symbol("eval", v)->bind;
}

value_t rl_eval(value_t v) {
  if ( is_symbol(v) )
    return eval_symbol(v);

  return eval_literal(v);
}


// print implementations ------------------------------------------------------
static void print_number(value_t val) {
  printf("%g", to_number("print", val));
}

static void print_unit(value_t val) {
  (void)val;
  printf("nul");
}

static void print_symbol(value_t val) {
  printf("%s", to_symbol("print", val)->name);
}

static void print_list(value_t val) {
  printf("(");

  list_t* xs = to_list("print", val);

  for ( ; xs->arity > 0; xs=xs->tail ) {
    rl_print(xs->head);

    if ( xs->arity > 1 )
      printf(" ");
  }

  printf(")");
}

static void print_value(value_t val) {
  printf("<%s @ %lx>", type_name(rl_type_of(val)), val);
}

void rl_print(value_t v) {
  type_t type = rl_type_of(v);

  switch (type) {
    case NUMBER: print_number(v); break;
    case UNIT:   print_unit(v); break;
    case SYMBOL: print_symbol(v); break;
    case LIST:   print_list(v); break;
    default:     print_value(v); break;
  }
}

// repl implementation --------------------------------------------------------
#define PROMPT  "rascal>"

void rl_repl(void) {
  while (true) {
    if (savepoint()) {
      reset_reader(true);
      newline();
      continue;
    }

    printf(PROMPT" ");
    value_t x = rl_read();
    value_t v = rl_eval(x);
    rl_print(v);
    newline();
  }
}

// main +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// version information --------------------------------------------------------
#define VERSION "%d.%d.%d.%c"
#define MAJOR 0
#define MINOR 0
#define PATCH 4
#define DEVELOPMENT 'a'

// prompts/messages -----------------------------------------------------------
#define WELCOME "Welcome to Rascal version "VERSION"!"
#define GOODBYE "Later blood!"

// startup helpers ------------------------------------------------------------
static void initialize_rascal(void) {
  init_reader(true);
}

static void finalize_rascal(void) {
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
