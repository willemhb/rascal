#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "debug.h"
#include "vm.h"
#include "interpreter.h"

// version information
#define MAJOR 0
#define MINOR 0
#define PATCH 4
#define DEV   "a"
#define VFMT "%.2d.%.2d.%.2d.%s"

static void welcomeMessage(void) {
  fprintf(stdout,
          "Welcome to Rascal version "VFMT"!\n",
          MAJOR,
          MINOR,
          PATCH,
          DEV);
}

static void goodbyeMessage(void) {
  fprintf(stdout, "Goodbye!\n");
}

static void initRascal(void) {
  toplevelInitObjects();
  initVm(&vm);
  welcomeMessage();
}

static void finalizeRascal(void) {
  freeVm(&vm);
  goodbyeMessage();
}

int main(const int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  initRascal();
  // runFile("/home/willem/Programming/C/rascal/rascal/base.rascal");
  repl(&vm);
  finalizeRascal();

  return 0;
}
