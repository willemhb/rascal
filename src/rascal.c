#include <stdio.h>
#include <getopt.h>

#include "runtime.h"

#include "lang/eval.h"
#include "lang/exec.h"

#include "val/list.h"
#include "val/text.h"
#include "val/table.h"

/* Globals */
#define RASCAL_LISP_DIR            "/home/willem/Programming/C/rascal/lisp"
#define RASCAL_OPTS                "hdrm:"
#define RASCAL_HELP_MSG            "No luck cowboy."
#define RASCAL_VERSION_FMT         "%.2d.%.2d.%.2d.%s"
#define RASCAL_MAJOR_VERSION       0
#define RASCAL_MINOR_VERSION       1
#define RASCAL_PATCH_VERSION       1
#define RASCAL_DEVELOPMENT_VERSION "a"

/* Initialization */
static void init_rascal_vm(void) {}

static void free_rascal_vm(void) {}

static void print_rascal_welcome(void) {
  fprintf(stdout,
          "Welcome to rascal version %.2d.%.2d.%2d.%s!\n",
          RASCAL_MAJOR_VERSION,
          RASCAL_MINOR_VERSION,
          RASCAL_PATCH_VERSION,
          RASCAL_DEVELOPMENT_VERSION);
}

static void print_rascal_goodbye(void) {
  fprintf(stdout, "Leaving Rascal, everything's fine :B\n. Later cowboy!\n\n");
}

static void add_to_args_buffer(char* a, size_t* n, Val* b) {
  b[(*n)++] = tag(mk_str(a, 0));
}

static void add_to_flags_buffer(char* a, size_t* n, Val* b) {
  b[(*n)++] = tag(mk_str(a, 0));
}

static void add_to_opts_buffer(char* o, char* a, size_t* n, Val* b) {
  b[(*n)++] = tag(mk_str(o, 0));
  b[(*n)++] = tag(mk_str(a, 0));
}

static void read_rascal_args(int argc, char* argv[argc], List** args, Set** flags, Map** opts) {
  static struct option long_options[] = {
    { "help",  no_argument,       0, 'h' },
    { "debug", no_argument,       0, 'd' },
    { "repl",  no_argument,       0, 'r' },
    { "main",  required_argument, 0, 'm' }
  };

  int option_index = 0;

  /* buffers */
  size_t na = 0;
  Val ab[argc];

  size_t nf = 0;
  Val fb[argc];

  size_t no = 0;
  Val ob[argc];

  while ( true ) {
    int c = getopt_long(argc, argv, RASCAL_OPTS, long_options, &option_index);

    if ( c == -1 )
      break;

    switch ( c ) {
      case 'h':
        add_to_flags_buffer("help", &nf, fb);
        break;

      case 'd':
        add_to_flags_buffer("debug", &nf, fb);
        break;

      case 'r':
        add_to_flags_buffer("repl", &nf, fb);
        break;

      case 'm':
        add_to_opts_buffer("main", optarg, &no, ob);
        break;
    }
  }

  /* Process remaining non-option command line arguments. */
  while ( optind < argc )
    add_to_args_buffer(argv[optind++], &na, ab);

  /* Create parsed Rascal objects for command line inputs. */
  *args  = mk_list(na, ab);
  *flags = mk_set(nf, fb);
  *opts  = mk_map(no, ob);
}

int main(int argc, char* argv[argc]) {
  init_rascal_vm();
  read_rascal_args(argc, argv, &ClArgs, &ClFlags, &ClOpts);
  rl_load(RASCAL_LISP_DIR "/" "boot.rl");

  // TODO: interpret command line arguments
  print_rascal_welcome();
  rl_repl();
  print_rascal_goodbye();
  free_rascal_vm();

  return 0;
}
