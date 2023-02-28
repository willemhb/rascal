#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "io.h"
#include "number.h"
#include "htable.h"
#include "memory.h"
#include "error.h"

// C types --------------------------------------------------------------------
typedef value_t (*reader)(int dispatch, FILE* ios);

typedef enum {
  ready_token,
  expr_token,
  eof_token,
  error_token
} token_t;

// globals --------------------------------------------------------------------
int EOS = EOF;

#define MIN_BUFFER    512
#define MIN_SUBEXPR   8
#define MIN_READTABLE 8

buffer_t Buffer         = { NULL, 0, MIN_BUFFER    };
values_t Subexpr        = { NULL, 0, MIN_SUBEXPR   };
htable_t Reader         = { NULL, 0, MIN_READTABLE };
htable_t DispatchReader = { NULL, 0, MIN_READTABLE };

token_t  Token   = ready_token;
value_t  Expr    = NUL;

#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define DIGIT  "0123456789"
#define SYMCHR "$%&^#:?!+-_*/<=>."

// internal helpers -----------------------------------------------------------
value_t read_expr(FILE* ios);

static void reset_reader(bool total) {
  free_buffer(&Buffer);
  Token = ready_token;
  Expr  = NUL;

  if (total)
    free_values(&Subexpr);
}

static usize accumulate(int ch) {
  return buffer_push(&Buffer, ch);
}

static char* token(void) {
  return Buffer.array;
}

static value_t take(bool total) {
  value_t expr = Expr;

  reset_reader(total);

  return expr;
}

static value_t give(value_t val, token_t token) {
  assert(Token == ready_token);
  Token = token;
  Expr  = val;
  return Expr;
}

static bool isrlspace(int ch) {
  return isspace(ch) || ch == '\t' || ch == ',';
}

static bool issymchar(int ch) {
  return isalnum(ch) || strchr("$%&^#:?!+-_*/<=>.", ch);
}

static void show_readtable(void) {
  printf("Readtable:\n\n");

  for (usize i=0; i<Reader.cap; i++) {
    int    key = *(int*)&Reader.table[i*2];
    reader val = *(reader*)&Reader.table[i*2+1];

    if (key == EOF)
      continue;
    
    if ((isspace(key) && key != ' ') || key=='\t')
      printf("[%3zu] %x = %p\n", i, key, val);

    else
      printf("[%3zu] %c = %p\n", i, key, val);
  }
}

static usize read_sequence(FILE* ios, char* type, int term, bool (*test)(value_t x)) {
  int ch;

  usize n = 0;

  while ((ch=fpeekc(ios)) != term) {
    if (ch == EOF) {
      raise_error(READ_ERROR, NUL, "unexpected EOF reading %s", type);
      break;
    }

    value_t expr = read_expr(ios);

    if (panicking())
      break;

    if (test && !test(expr)) {
      raise_error(READ_ERROR, NUL, "test failed reading %s", type);
      break;
    }

    values_push(&Subexpr, expr);
    n++;
  }

  if (panicking()) {
    values_popn(&Subexpr, n);
    return 0;
  }

  fgetc(ios); // clear terminal

  return n;
}

// API ------------------------------------------------------------------------
int newln(void) {
  return fnewln(stdin);
}


int fnewln(FILE* ios) {
  return fprintf(ios, "\n");
}

int peekc(void) {
  return fpeekc(stdin);
}

int fpeekc(FILE* ios) {
  int out = fgetc(ios);

  if (out != EOF)
    ungetc(out, ios);

  return out;
}

// interpreter ----------------------------------------------------------------
void print_real(value_t val)   { printf("%g", as_real(val));                 }
void print_fixnum(value_t val) { printf("%lu", as_fixnum(val));              }
void print_symbol(value_t val) { printf("%s", as_symbol(val)->name);         }
void print_unit(value_t val)   { (void)val; printf("nul");                   }
void print_bool(value_t val)   { printf(val == TRUE_VAL ? "true" : "false"); }
void print_native(value_t val) { (void)val; printf("#'native");              }

void print_list(value_t val) {
  printf("(");

  list_t* xs = as_list(val);

  while (xs->len) {
    print(xs->head);

    xs = xs->tail;

    if (xs->len)
      printf(" ");
  }

  printf(")");
}

void print_binary(value_t val) {
  printf("#\"");

  binary_t* bs = as_binary(val);

  for (usize i=0; i<bs->len; i++) {
    printf("%.3d", bs->array[i]);

    if (i+i < bs->len)
      printf(" ");
  }

  printf("\"");
}

void (*Print[])(value_t val) = {
  [REAL]   = print_real,
  [FIXNUM] = print_fixnum,
  [UNIT]   = print_unit,
  [BOOL]   = print_bool,
  [NATIVE] = print_native,
  [SYMBOL] = print_symbol,
  [LIST]   = print_list,
  [BINARY] = print_binary
};

value_t read_expr(FILE* ios) {
  int ch;
  reader readfn;

  while (!Token && !panicking()) {
    ch     = fgetc(ios);
    readfn = (reader)reader_get(&Reader, ch);

    if (readfn == NULL)
      raise_error(READ_ERROR, NUL, "unrecognized dispatch '%c'", (char)ch);

    else
      readfn(ch, ios);
  }

  return take(panicking());
}

value_t read(void) {
  reset_reader(true);

  value_t out = read_expr(stdin);

  return out;
}

void print(value_t val) {
  Print[type_of(val)](val);
}

value_t read_error(value_t expr) {
  return give(expr, error_token);
}

// reader dispatches ----------------------------------------------------------
value_t read_space(int ch, FILE* ios) {
  while (isrlspace(ch)) {
    ch = fgetc(ios);
  }

  assert(!isrlspace(ch));

  if (ch != EOF)
    ungetc(ch, ios);

  return NUL;
}

value_t read_eof(int ch, FILE* ios) {
  (void)ch;
  (void)ios;

  give(NUL, eof_token);
  return NUL;
}

value_t read_symbol(int ch, FILE* ios) {
  accumulate(ch);

  while (issymchar(ch=fgetc(ios)))
    accumulate(ch);

  if (ch != EOF)
    ungetc(ch, ios);

  char* t = token();

  if (strcmp(t, "nul") == 0)
    return give(NUL, expr_token);

  else if (strcmp(t, "true") == 0)
    return give(TRUE_VAL, expr_token);

  else if (strcmp(t, "false") == 0)
    return give(FALSE_VAL, expr_token);

  else
    return give(symbol(t), expr_token);
}

value_t read_number(int ch, FILE* ios) {
  accumulate(ch);

  while (issymchar(ch=fgetc(ios)))
    accumulate(ch);

  if (ch != EOF)
    ungetc(ch, ios);

  char* t  = token(),* tend;

  if (strchr(t, '.')) {
    real_t r = strtod(t, &tend);

    if (*tend != '\0')
      return give(symbol(t), expr_token);

  else
    return give(tag_dbl(r), expr_token);
  } else {
    fixnum_t f = strtol(t, &tend, 10);

    if (*tend != '\0')
      return give(symbol(t), expr_token);

    else
      return give(tag_word(f, FIXNUMTAG), expr_token);
  }
}

value_t read_list(int ch, FILE* ios) {
  (void)ch;

  usize n = read_sequence(ios, "list", ')', NULL);
  repanic(NUL);
  value_t x = list(n, &Subexpr.array[Subexpr.len-n]);
  values_popn(&Subexpr, n);
  return give(x, expr_token);
}

value_t read_binary(int ch, FILE* ios) {
  (void)ch;

  usize n = read_sequence(ios, "binary", '"', is_byte);
  repanic(NUL);
  value_t x = binary(n, &Subexpr.array[Subexpr.len-n]);
  values_popn(&Subexpr, n);
  return give(x, expr_token);
}

value_t read_dispatch(int ch, FILE* ios) {
  reader readfn;

  while (!Token && !panicking()) {
    ch     = fgetc(ios);
    readfn = (reader)reader_get(&DispatchReader, ch);

    if (readfn == NULL)
      raise_error(READ_ERROR, NUL, "unrecognized dispatch '%c'", (char)ch);

    else
      readfn(ch, ios);
  }

  return Expr;
}

// initialization -------------------------------------------------------------
static void add_dispatch_reader(int ch, reader handler) {
  reader_set(&DispatchReader, ch, (funcptr)handler);
}

static void add_reader(int ch, reader handler) {
  reader_set(&Reader, ch, (funcptr)handler);
  // show_readtable();
}

static void add_readers(char* chs, reader handler) {
  for (;(*chs) != '\0'; chs++) {
    add_reader(*chs, handler);
  }
}

void reader_init(void) {
  // initialize global state --------------------------------------------------
  init_buffer(&Buffer);
  init_values(&Subexpr);
  init_reader(&Reader);
  init_reader(&DispatchReader);

  // add readers --------------------------------------------------------------
  // toplevel -----------------------------------------------------------------
  add_readers(" \t\n\v\f\r,", read_space);
  add_readers("0123456789", read_number);
  add_readers(UPPER,  read_symbol);
  add_readers(LOWER,  read_symbol);
  add_readers(SYMCHR, read_symbol);
  add_reader('(', read_list);
  add_reader('#', read_dispatch);
  add_reader(EOF, read_eof);

  // dispatch -----------------------------------------------------------------
  add_dispatch_reader('"', read_binary);

  // show_readtable();
}
