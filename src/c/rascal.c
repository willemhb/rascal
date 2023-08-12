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

// internal types -------------------------------------------------------------
typedef enum type_t {
  NOTYPE,
  NUMBER
} type_t;

typedef struct {
  jmp_buf jmpbuf;
} vm_t;

// value types ----------------------------------------------------------------
typedef uintptr_t value_t;
typedef double number_t;

// tags/masks +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define QNAN 0x7ff8000000000000UL
#define SIGN 0x8000000000000000UL
#define TAGM 0xffff000000000000UL
#define VALM 0x0000ffffffffffffUL

// size limits ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAXPOW2 0x8000000000000000UL
#define MAXSIZE 0x0000ffffffffffffUL

// forward declarations +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// runtime implementations ----------------------------------------------------
void* allocate(size_t n, bool fromHeap);
void* reallocate(void* ptr, size_t oldN, size_t newN, bool fromHeap);
void* deallocate(void* ptr, size_t n, bool fromHeap);

// type implementations -------------------------------------------------------
// number type ----------------------------------------------------------------
number_t to_number(const char* fname, value_t value);
value_t mk_number(number_t number);
bool is_number(value_t x);

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
  size_t type_##push(ctype* array, eltype element);                    \
  size_t type_##write(ctype* array, size_t n, eltype* elements);       \
  eltype type_##pop(ctype* array)

#define ARRAY_IMPL(type, ctype, eltype, encoded)                       \
  void init_##type(ctype* array) {                                     \
    array->cnt  = 0;                                                   \
    array->cap  = 0;                                                   \
    array->data = NULL;                                                \
  }                                                                    \
                                                                       \
  void free_##type(ctype* array) {                                     \
    array->;                                                           \
  }                                                                    \
                                                                       \
  void reset_##type(ctype* array);                                     \
  size_t resize_##type(ctype* array, size_t n);                        \
  size_t type_##push(ctype* array, eltype element);                    \
  size_t type_##write(ctype* array, size_t n, eltype* elements);       \
  eltype type_##pop(ctype* array)


// global variables +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
vm_t Vm;

// implementations ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
  longjmp(Vm.jmpbuf, 1);
}

value_t rl_read(void);

value_t rl_eval(value_t v) {
  return v;
}

void rl_print(value_t v) {
  type_t type = rl_type_of(v);
  switch (type) {
    case NUMBER: printf("%.2g", as_number(v)); break;
    default:     printf("#<%s @ %lx>", type_name(type), v); break;
  }
}

#define PROMPT  "rascal>"

void rl_repl(void) {
  while (true) {
    if (setjmp(Vm.jmpbuf)) {
      printf("\n");
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
