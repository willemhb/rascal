#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "io.h"
#include "number.h"
#include "htable.h"
#include "memory.h"

// C types --------------------------------------------------------------------
typedef void (*reader)(int dispatch, FILE* ios);

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

buffer_t Buffer  = { NULL, 0, MIN_BUFFER    };
values_t Subexpr = { NULL, 0, MIN_SUBEXPR   };
htable_t Reader  = { NULL, 0, MIN_READTABLE };
token_t  Token   = ready_token;
value_t  Expr    = NUL;

#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define DIGIT  "0123456789"
#define SYMCHR "$%&^#:?!+-_*/<=>."

// internal helpers -----------------------------------------------------------
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

static void give(value_t val, token_t token) {
  assert(Token == ready_token);
  Token = token;
  Expr  = val;
}

static void subexpr(void) {
  value_t expr = take(false);
  values_push(&Subexpr, expr);
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
void print_real(value_t val)   { printf("%g", as_real(val));         }
void print_symbol(value_t val) { printf("%s", as_symbol(val)->name); }
void print_unit(value_t val)   { (void)val; printf("nul");           }

void (*Print[])(value_t val) = {
  [REAL]   = print_real,
  [UNIT]   = print_unit,
  [SYMBOL] = print_symbol
};

value_t read_expr(FILE* ios) {
  int ch;
  reader readfn;

  while (!Token) {
    ch     = fgetc(ios);
    readfn = (reader)reader_get(&Reader, ch);
    // assert(readfn != NULL);
    readfn(ch, ios);
  }

  return take(true);
}

value_t read(void) {
  reset_reader(true);

  value_t out = read_expr(stdin);

  return out;
}

void print(value_t val) {
  Print[type_of(val)](val);
}

// reader dispatches ----------------------------------------------------------
void read_space(int ch, FILE* ios) {
  while (isrlspace(ch))
    ch = fgetc(ios);

  if (ch != EOF)
    ungetc(ch, ios);
}

void read_eof(int ch, FILE* ios) {
  (void)ch;
  (void)ios;

  give(NUL, eof_token);
}

void read_symbol(int ch, FILE* ios) {
  accumulate(ch);

  while (issymchar((ch=fgetc(ios))))
    accumulate(ch);

  if (ch != EOF)
    ungetc(ch, ios);

  char* t = token();

  if (strcmp(t, "nul") == 0)
    give(NUL, expr_token);

  else
    give(symbol(t), expr_token); 
}

void read_number(int ch, FILE* ios) {
  accumulate(ch);

  while (issymchar((ch=fgetc(ios))))
    accumulate(ch);

  char* t  = token(),* tend;
  real_t r = strtod(t, &tend);

  if (*tend != '\0')
    give(symbol(t), expr_token);

  else
    give(tag_dbl(r), expr_token);
}

// initialization -------------------------------------------------------------
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

  // add readers --------------------------------------------------------------
  add_readers(" \t\n\v\f\r,", read_space);
  add_readers("0123456789", read_number);
  add_readers(UPPER,  read_symbol);
  add_readers(LOWER,  read_symbol);
  add_readers(SYMCHR, read_symbol);
  add_reader(EOF, read_eof);

  // show_readtable();
}
