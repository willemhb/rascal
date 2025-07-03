#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#include "data/data.h"
#include "lang/lang.h"
#include "sys/sys.h"

// setup ----------------------------------------------------------------------
void print_welcome(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  print_welcome();
}

// teardown -------------------------------------------------------------------
void teardown(void) {}

// rascal! --------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  setup();
  repl();
  teardown();

  return 0;
}
