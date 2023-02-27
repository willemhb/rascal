#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "value.h"
#include "memory.h"
#include "number.h"
#include "io.h"

// interpreter ----------------------------------------------------------------

value_t eval(value_t val) {
  return val;
}

#define PROMPT "rascal>"

void repl(void) {
  for (;;) {
    printf(PROMPT" ");
    value_t x = read();
    value_t v = eval(x);

#ifdef RASCAL_DEBUG
    printf("%s> ", type_name_of(v));
#endif

    print(v);
    printf("\n");
  }
}

// startup --------------------------------------------------------------------
#define MAJOR       0
#define MINOR       0
#define DEVELOPMENT 1
#define PATCH       "a"
#define VERSION     "%d.%d.%d.%s"

extern void reader_init(void);
extern void memory_init(void);

void startup(void) {
  memory_init();
  reader_init();
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
