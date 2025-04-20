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
}

void print_welcome(void) {
  
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  define_builtins();
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
