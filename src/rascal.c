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

    print(v);
    newln();
  }
}

// startup --------------------------------------------------------------------
#define MAJOR       0
#define MINOR       0
#define DEVELOPMENT 1
#define PATCH       "a"
#define VERSION     "%d.%d.%d.%s"

void startup(void) {
  printf("Welcome to rascal version "VERSION"!", MAJOR, MINOR, DEVELOPMENT, PATCH);
  newln();
}

// shutdown -------------------------------------------------------------------

// main -----------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;

  startup();
  repl();

  return 0;
}
