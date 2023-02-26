#include <stdio.h>
#include <stdlib.h>

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
#define MIN_BUFFER 512

struct {
  char *chars;
  usize len, cap;
} Buffer = {
  NULL,
  0,
  MIN_BUFFER
};

struct {
  
};

token_t Token = ready_token;

htable_t Readtable;
// internal helpers -----------------------------------------------------------
reader get_reader(int ch) {
  return (reader)reader_get(&Readtable, ch);
}

void set_reader(int ch, reader dispatch) {
  reader_set(&Readtable, ch, (funcptr)dispatch);
}

void set_readers(const char* chs, reader dispatch) {
  for (const char* ch=chs; *ch !='\0'; ch++)
    set_reader(*ch, dispatch);
}

void init_buffer(void) {
  Buffer.len   = 0;
  Buffer.cap   = pad_array_size(0, 0, MIN_BUFFER, 1.0);
  Buffer.chars = allocate(Buffer.cap*sizeof(char));
}

void reset_buffer(void) {
  deallocate(Buffer.chars, Buffer.cap*sizeof(char));
  init_buffer();
}

void resize_buffer(usize n) {
  usize newcap = pad_array_size(n, Buffer.cap, MIN_BUFFER, 1.0);

  if (newcap != Buffer.cap) {
    Buffer.chars = reallocate(Buffer.chars, newcap*sizeof(char), Buffer.cap*sizeof(char));
    Buffer.cap   = newcap;
  }
}

void buffer_write(char ch) {
  resize_buffer(Buffer.len+1);
  Buffer.chars[Buffer.len++] = ch;
}

void reset_reader(void) {
  reset_buffer();
  Token = ;
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
  
}

// interpreter ----------------------------------------------------------------
void print_real(value_t val)   { printf("%g", as_real(val));         }
void print_symbol(value_t val) { printf("%s", as_symbol(val)->name); }
void print_unit(value_t val)   { }

void (*Print[])(value_t val) = {
  [REAL]   = print_real,
  [UNIT]   = print_unit,
  [SYMBOL] = print_symbol
};

value_t read(void) {
  reset_buffer();
  
  while (!Token) {
    int ch   = peekc();
    reader r = 
  }
}

void print(value_t val) {
  Print[type_of(val)](val);
}
