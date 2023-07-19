// includes +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// standard includes ----------------------------------------------------------
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>

// core includes --------------------------------------------------------------
#include "common.h"
#include "object.h"
#include "runtime.h"
#include "interp.h"
#include "native.h"
#include "lang.h"

// library includes -----------------------------------------------------------
#include "util/string.h"
#include "util/number.h"
#include "util/hashing.h"
#include "util/io.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define VERSION "%d.%d.%d.%c"
#define MAJOR 0
#define MINOR 1
#define PATCH 0
#define DEV   'a'

// initialization/finalization ++++++++++++++++++++++++++++++++++++++++++++++++
void init_rascal( void ) {
  toplevel_init_runtime();
  toplevel_init_object();
  toplevel_init_lang();
  toplevel_init_interp();
  toplevel_init_native();
  
  printf("Welcome to rascal version "VERSION"!\n", MAJOR, MINOR, PATCH, DEV);
}

void process_args( int argc, const char* argv[argc] ) {
  (void)argv;
}

void finalize_rascal( void ) {
  printf("goodbye!\n");
}

// main +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, const char* argv[argc]) {
  init_rascal();
  process_args(argc, argv);
  repl();
  finalize_rascal();
  return 0;
}
