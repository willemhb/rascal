#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#include "data/data.h"
#include "sys/sys.h"
#include "lang/lang.h"

// setup ----------------------------------------------------------------------
void print_welcome(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  toplevel_init_data();
  toplevel_init_sys();
  toplevel_init_lang();
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
