#include "common.h"
#include "debug.h"
#include "memory.h"
#include "value.h"
#include "object.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "vm.h"

// version information
#define MAJOR 0
#define MINOR 0
#define PATCH 1
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

static void initRascal(void) {
  toplevelInitObjects();
  
}

int main(const int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  return 0;
}
