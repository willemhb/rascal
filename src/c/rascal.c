#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// startup --------------------------------------------------------------------
#define MAJOR       0
#define MINOR       0
#define DEVELOPMENT 5
#define PATCH       "a"
#define VERSION     "%d.%d.%d.%s"

void startup(void) {
  memory_init();
  compare_init();
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
