#include "io.h"

// API ------------------------------------------------------------------------
int newln(void) {
  return printf("\n");
}

char* getln(void) {
  static char buffer[4096];
  fgets(buffer, 4096, stdin);

  return buffer;
}
