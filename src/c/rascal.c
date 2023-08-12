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

typedef struct {
  char* data;
  size_t cnt, cap;
} text_buffer_t;

// internal enum types --------------------------------------------------------
typedef enum type_t {
  NOTYPE,
  NUMBER,
  UNIT
} type_t;

typedef enum token_t {
  READY,
  EXPRESSION,
  END_OF_STREAM
} token_t;

// value types ----------------------------------------------------------------
typedef uintptr_t value_t;
typedef double number_t;

// internal structure types ---------------------------------------------------
typedef struct {
  // reader -------------------------------------------------------------------
  struct {
    value_t expression;
    size_t buffer_size;
    char* buffer;
  } reader;

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

// size limits ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAXPOW2 0x8000000000000000UL
#define MAXSIZE 0x0000ffffffffffffUL

// forward declarations +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// runtime implementations ----------------------------------------------------
void* allocate(size_t n, bool fromHeap);
void* allocate_array(size_t n, size_t o, bool fromHeap);
void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap);
void* reallocate_array(void* ptr, size_t oldN, size_t newN, size_t o, bool fromHeap);
void* deallocate(void* ptr, size_t n, bool fromHeap);
void* deallocate_arraY(void* ptr, size_t n, size_t o, bool fromHeap);

// type implementations -------------------------------------------------------
// number type ----------------------------------------------------------------
number_t to_number(const char* fname, value_t value);
value_t mk_number(number_t number);
bool is_number(value_t x);

// unit type ------------------------------------------------------------------
bool is_unit(value_t x);

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v);

// interpreter functions ------------------------------------------------------
void rl_error(const char* fname, const char* fmt, ...);
value_t rl_read(void);
value_t rl_eval(value_t v);
void rl_print(value_t v);
void rl_repl(void);

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

static const char* type_name(type_t type) {
  const char* out;
  
  switch (type) {
    case NUMBER: out = "number"; break;
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

// implementations ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// array implementations ------------------------------------------------------
ARRAY_API(values, values_t, value_t);
ARRAY_IMPL(values, values_t, value_t, false);
ARRAY_API(text_buffer, text_buffer_t, char);
ARRAY_IMPL(text_buffer, text_buffer_t, char, true);

// type implementations -------------------------------------------------------
// number type ----------------------------------------------------------------
SAFECAST(number_t, number, as_number);

value_t mk_number(number_t number) {
  return ((ieee754_64_t)number).word;
}

bool is_number(value_t x) {
  return (x & QNAN) != QNAN;
}

// internal functions ---------------------------------------------------------
type_t rl_type_of(value_t v) {
  switch (v & VALM) {
    default: return NUMBER;
  }
}

// interpreter functions ------------------------------------------------------
void rl_error(const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s: error: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);
  longjmp(Vm.error.jmpbuf, 1);
}

#define READER_BUFFER_SIZE 512

static void init_reader(void) {
  Vm.reader.expression = NUL;
  Vm.reader.buffer_size = READER_BUFFER_SIZE;
  Vm.reader.buffer = allocate_array(READER_BUFFER_SIZE, , false);
}

static void free_reader(void) {
  
}

static void reset_reader(FILE* src) {
  
}

static char* rl_getline(FILE* src) {
  
}

value_t rl_read(void) {
  reset_reader(stdin);
  char* token = rl_getline(Vm.source);

  if (strcmp(token, "nul") == 0) {
    Vm.expression = NUL;
    Vm.token = EXPRESSION;
  }
}

value_t rl_eval(value_t v) {
  return v;
}

void rl_print(value_t v) {
  type_t type = rl_type_of(v);
  switch (type) {
    case NUMBER: printf("%.2g", as_number(v)); break;
    case UNIT:   printf("nul"); break;
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
    newline();
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
#define PATCH 2
#define DEVELOPMENT 'a'

// prompts/messages -----------------------------------------------------------
#define WELCOME "Welcome to Rascal version "VERSION"!"
#define GOODBYE "Later blood!"

// startup helpers ------------------------------------------------------------
static void initialize_rascal(void) {}

static void finalize_rascal(void) {}

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
