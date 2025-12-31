#include "common.h"
#include "util/collection.h"
#include "util/util.h"
#include "val.h"
#include "vm.h"
#include "lang.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

// command line options -------------------------------------------------------
static struct option long_options[] = {
  {"help",    no_argument, 0, 'h'},
  {"version", no_argument, 0, 'v'},
  {"debug",   no_argument, 0, 'd'},
  {0, 0, 0, 0}
};

void print_help(const char* progname) {
  printf("Usage: %s [OPTIONS] [FILE]\n", progname);
  printf("\n");
  printf("A bytecode interpreter for a small Lisp dialect.\n");
  printf("\n");
  printf("Arguments:\n");
  printf("  FILE        Execute the specified file\n");
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help     Show this help message and exit\n");
  printf("  -v, --version  Show version information and exit\n");
  printf("\n");
  printf("If no FILE is provided, an interactive REPL is started.\n");
}

void print_version(void) {
  printf("rascal version " VERSION "\n", MAJOR, MINOR, PATCH, RELEASE);
}

// setup/teardown -------------------------------------------------------------
void init_builtin_types(void) {
  register_builtin_types(&Main);
}

void init_standard_streams(void) {
  // add FILE* objects to corresponding standard ports ------------------------
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;
}

void init_static_objects(void) {
  // register static global objects so they're handled correctly by gc
  add_to_permanent(&Main, Vm.globals);
  add_to_permanent(&Main, &Ins);
  add_to_permanent(&Main, &Outs);
  add_to_permanent(&Main, &Errs);
}

void define_globals(void) {
  // initialize other globals
  toplevel_env_def(&Main, Vm.globals, mk_sym(&Main, "&ins"), tag_obj(&Ins), false, true);
  toplevel_env_def(&Main, Vm.globals, mk_sym(&Main, "&outs"), tag_obj(&Outs), false, true);
  toplevel_env_def(&Main, Vm.globals, mk_sym(&Main, "&errs"), tag_obj(&Errs), false, true);
  toplevel_env_def(&Main, Vm.globals, mk_sym(&Main, "&globals"), tag_obj(Vm.globals), false, true);
}

void init_vm(void) {
  Vm.initialized = true;
}

void init_standard_library(void) {
  // load the standard library file
  save_error_state(&Main, 0);

  if ( rl_setjmp(&Main) ) {
    printf("failed to load lisp/boot.rl");
    restore_error_state(&Main);
  } else {
    load_file(&Main, "lisp/boot.rl");
    load_file(&Main, "lisp/bquote.rl");
    load_file(&Main, "lisp/core.rl");
    load_file(&Main, "lisp/oslib.rl");
    load_file(&Main, "lisp/math.rl");
  }

  discard_error_state(&Main);

#ifdef RASCAL_DEBUG
  save_error_state(&Main, 0);

  if ( rl_setjmp(&Main) ) {
    printf("failed to load lisp/test.rl");
    restore_error_state(&Main);
  } else {
    load_file(&Main, "lisp/test.rl");
  }

  discard_error_state(&Main);
#endif 
}

void print_welcome(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  init_builtin_types();
  init_standard_streams();
  init_static_objects();
  init_vm_error();
  define_builtins();
  define_globals();
  init_vm();
  init_standard_library();
}

void teardown(void) {}

// entry point
int main(int argc, char* argv[]) {
  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1) {
    switch (opt) {
      case 'h':
        print_help(argv[0]);
        return 0;
      case 'v':
        print_version();
        return 0;
      default:
        print_help(argv[0]);
        return 1;
    }
  }

  setup();

  if (optind < argc) {
    // Execute the specified file
    load_file(&Main, argv[optind]);
  } else {
    // Enter REPL
    print_welcome();
    toplevel_repl(&Main);
  }

  teardown();

  return 0;
}
