#include "util/io.h"

#include "debug.h"
#include "vm.h"

// version information
#define MAJOR 0
#define MINOR 0
#define PATCH 2
#define DEV   "a"
#define VFMT "%.2d.%.2d.%2d.%s"

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

static void lexFile(const char* fname) {
  char* source = readFile(fname);
  scan(&vm.scanner, source);
  displayScanner(&vm.scanner);
}

int main(const int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  initRascal();
  lexFile("/home/willem/Programming/C/rascal/rascal/base.rascal");
  finalizeRascal();

  return 0;
}
