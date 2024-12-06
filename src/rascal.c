#include <stdio.h>
#include <getopt.h>

#include "runtime.h"

#include "lang/eval.h"
#include "lang/exec.h"

#include "val/list.h"
#include "val/alist.h"
#include "val/text.h"
#include "val/table.h"

#include "vm/state.h"

/* Globals */
#define RL_LISP_DIR "/home/willem/Programming/C/rascal/lisp"
#define RL_OPTS     "hdrm:"
#define RL_HELP     "No luck cowboy."
#define RL_VERSION  "%.2d.%.2d.%.2d.%s"
#define RL_MAJOR     0
#define RL_MINOR     1
#define RL_PATCH     1
#define RL_DEV      "a"

/* Initialization */
static void rl_init(void) {
  rl_toplevel_init_state();
  rl_toplevel_init_types();
}

static void rl_free(void) {
  
}

static void rl_welcome(void) {
  fprintf(stdout, "Welcome to rascal version " RL_VERSION "!\n",
          RL_MAJOR, RL_MINOR, RL_PATCH, RL_DEV );
}

static void rl_goodbye(void) {
  fprintf(stdout, "Leaving Rascal, everything's fine B-)\n. Later cowboy!\n\n");
}

int main(int argc, char* argv[argc]) {
  (void)argv;

  init_rl();

  // TODO: interpret command line arguments
  rl_welcome();
  rl_repl(&Vm);
  rl_goodbye();
  free_rl();

  return 0;
}
