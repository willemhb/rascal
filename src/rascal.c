#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "common.h"
#include "value.h"
#include "memory.h"
#include "number.h"
#include "io.h"
#include "native.h"
#include "error.h"

// startup --------------------------------------------------------------------
#define MAJOR       0
#define MINOR       0
#define DEVELOPMENT 5
#define PATCH       "a"
#define VERSION     "%d.%d.%d.%s"

void startup(void) {
  memory_init();
  reader_init();
  error_init();
  native_init();
  eval_init();

  printf("Welcome to rascal version "VERSION"!\n\n", MAJOR, MINOR, DEVELOPMENT, PATCH);
}

// shutdown -------------------------------------------------------------------
void shutdown(void) {
  printf("Exiting rascal normally.\n");
}

// main -----------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;

  startup();
  repl();
  shutdown();

  return 0;
}
