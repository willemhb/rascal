#include "runtime.h"
#include "lang.h"
#include "interpreter.h"
#include "value.h"
#include "util.h"

/* Globals */
#define RASCAL_LISP_DIR "/home/willem/Programming/C/rascal/lisp/"

/* Initialization */
static void init_rascal_vm(void) {}

static void free_rascal_vm(void) {}

static void read_rascal_args(int argc, const char* argv[argc], List** args, Set** flags, Map** opts) {
  (void)argc;
  (void)argv;
  (void)args;
  (void)flags;
  (void)opts;
}

int main(const int argc, const char* argv[argc]) {
  init_rascal_vm();

  read_rascal_args(argc,argv, &ClArgs, &ClFlags, &ClOpts);

  rl_load(RASCAL_LISP_DIR "boot.rl");

  // interpret command line arguments
  rl_repl();

  free_rascal_vm();

  return 0;
}
