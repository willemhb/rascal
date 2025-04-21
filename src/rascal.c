#include "common.h"
#include "collection.h"
#include "util.h"
#include "data.h"
#include "runtime.h"
#include "lang.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// setup/teardown
void define_builtins(void) {
  def_builtin_fun("+", OP_ADD);
  def_builtin_fun("-", OP_SUB);
  def_builtin_fun("*", OP_MUL);
  def_builtin_fun("/", OP_DIV);
  def_builtin_fun("=", OP_EGAL);
  def_builtin_fun("type", OP_TYPE);

  // special forms and other syntactic markers
  QuoteStr = mk_str("quote");
  SetStr   = mk_str("set");
  IfStr    = mk_str("if");
  DoStr    = mk_str("do");
}

void initialize_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_TYPES; i++ ) {
    ExpTypeInfo* info = &Types[i]; strcpy(buffer+1, info->name);
    info->repr        = mk_sym(buffer);
  }
}

void print_welcome(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  define_builtins();
  initialize_types();
  print_welcome();
}

void teardown(void) {}

// entry point
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  setup();
  repl();
  teardown();

  return 0;
}
