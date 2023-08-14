#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

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

// array types ----------------------------------------------------------------
typedef struct {
  uintptr_t* data;
  size_t cnt, cap;
} values_t;

// internal enum types --------------------------------------------------------
typedef enum type_t {
  NOTYPE,
  NUMBER,
  UNIT,
  SYMBOL,
  LIST
} type_t;

// value types ----------------------------------------------------------------
typedef uintptr_t value_t;
typedef double number_t;
typedef struct object {
  type_t type;
} object_t;

// object types ---------------------------------------------------------------
typedef struct symbol {
  object_t obj;
  struct symbol* left, * right;
  char* name;
  value_t bind;
} symbol_t;

typedef struct list {
  object_t obj;
  size_t arity;
  value_t head;
  struct list* tail;
} list_t;

// internal structure types ---------------------------------------------------
typedef struct {
  // reader -------------------------------------------------------------------
  struct {
    size_t buffer_size;
    char* buffer;
  } reader;

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

// size limits ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAXPOW2 0x8000000000000000UL
#define MAXSIZE 0x0000ffffffffffffUL

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
value_t mk_number(number_t number);
bool is_number(value_t x);

// unit type ------------------------------------------------------------------
bool is_unit(value_t x);

// symbol type ----------------------------------------------------------------
symbol_t* to_symbol(const char* fname, value_t value);
value_t   mk_symbol(char* token);
bool      is_symbol(value_t x);

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
  
  switch (type) {
    case NUMBER: out = "number"; break;
    case UNIT:   out = "unit"; break;
    case SYMBOL: out = "symbol"; break;
    default:     out = "unknown"; break;
  }

  return out;
}

// misc utilities +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uint64_t ceilpow2(uint64_t i) {
  if (i == 0)
    return 1;

  if ((i & (i - 1)) == 0)
    return i;

  if (i & MAXPOW2)
    return MAXPOW2;

  while (i & (i - 1))
    i = i & (i - 1);

  return i << 1;
}

// describe macros ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SAFECAST(ctype, type, convert)                           \
  ctype to_##type(const char* fname, value_t val) {              \
    rl_require(is_##type(val), fname, "not a %s", #type);        \
    return (ctype)convert(val);                                  \
  }

#define ARRAY_API(type, ctype, eltype)                                 \
  void init_##type(ctype* array);                                      \
  void free_##type(ctype* array);                                      \
  void reset_##type(ctype* array);                                     \
  size_t resize_##type(ctype* array, size_t n);                        \
  size_t type##_push(ctype* array, eltype element);                    \
  size_t type##_write(ctype* array, size_t n, eltype* elements);       \
  eltype type##_pop(ctype* array)

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
  }

// global variables +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
vm_t Vm;

list_t EmptyList = {
  .obj={ LIST },
  .arity=0,
  .head=NUL,
  .tail=&EmptyList
};

// implementations ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// array implementations ------------------------------------------------------
ARRAY_API(values, values_t, value_t);
ARRAY_IMPL(values, values_t, value_t, false);

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
  free(ptr);
}

void  deallocate_array(void* ptr, size_t n, size_t o, bool fromHeap) {
  return deallocate(ptr, n*o, fromHeap);
}


// type implementations -------------------------------------------------------
// number type ----------------------------------------------------------------
SAFECAST(number_t, number, as_number);

value_t mk_number(number_t number) {
  return ((ieee754_64_t)number).word;
}

bool is_number(value_t x) {
  return (x & QNAN) != QNAN;
}

// unit type ------------------------------------------------------------------
SAFECAST(symbol_t*, symbol, as_pointer);

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
  symbol_t* out = allocate(sizeof(symbol_t), true);
  out->obj.type = SYMBOL;
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

bool is_symbol(value_t value) {
  return rl_type_of(value) == SYMBOL;
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
void rl_error(const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "\n%s: error: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);
  longjmp(Vm.error.jmpbuf, 1);
}

#define READER_BUFFER_SIZE 512

static void init_reader(void) {
  Vm.reader.buffer_size = READER_BUFFER_SIZE;
  Vm.reader.buffer      = allocate_array(READER_BUFFER_SIZE, sizeof(char), false);
}

static void free_reader(void) {
  deallocate_array(Vm.reader.buffer, Vm.reader.buffer_size, sizeof(char), false);
}

static void reset_reader(void) {
  free_reader();
  init_reader();
}

static char* rl_getline(char** buffer, size_t* size, FILE* stream) {
  ssize_t result = getline(buffer, size, stream);
  

  if ( result == -1 ) {
    fprintf(stderr, "<runtime @ getline>: error: %s.\n", strerror(ferror(stream)));
    exit(1);
  }

  size_t tokenLength = strlen(*buffer);
  (*buffer)[tokenLength-1] = '\0'; // remove delimiter
  return *buffer;
}

value_t rl_read(void) {
  reset_reader();

  value_t out;
  char* token = rl_getline(&Vm.reader.buffer, &Vm.reader.buffer_size, stdin);

  if ( strcmp(token, "nul") == 0 )
    out = NUL;

  else {
    char* str_end;
    number_t num = strtod(token, &str_end);

    if (*str_end=='\0')
      out = mk_number(num);

    else
      out = mk_symbol(token);
  }

  return out;
}

value_t rl_eval(value_t v) {
  if ( is_symbol(v) )
    return to_symbol("eval", v)->bind;

  return v;
}

void rl_print(value_t v) {
  type_t type = rl_type_of(v);
  switch (type) {
    case NUMBER: printf("%g", to_number("print", v)); break;
    case UNIT:   printf("nul"); break;
    case SYMBOL: printf("%s", to_symbol("print", v)->name); break;
    default:     printf("#<%s @ %lx>", type_name(type), v); break;
  }
}

#define PROMPT  "rascal>"

void rl_repl(void) {
  while (true) {
    if (savepoint()) {
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
#define PATCH 3
#define DEVELOPMENT 'a'

// prompts/messages -----------------------------------------------------------
#define WELCOME "Welcome to Rascal version "VERSION"!"
#define GOODBYE "Later blood!"

// startup helpers ------------------------------------------------------------
static void initialize_rascal(void) {
  init_reader();
}

static void finalize_rascal(void) {
  free_reader();
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
