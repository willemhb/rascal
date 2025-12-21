#include "common.h"
#include "collection.h"
#include "util.h"
#include "data.h"
#include "runtime.h"
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
void init_standard_streams(void) {
  // add FILE* objects to corresponding standard ports ------------------------
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;
}

void init_static_objects(void) {
  // register static global objects in heap -----------------------------------
  // so that they get unmarked during the sweep phase -------------------------
  add_to_heap(&Main, &Globals);
  add_to_heap(&Main, &Ins);
  add_to_heap(&Main, &Outs);
  add_to_heap(&Main, &Errs);
}

void define_builtins(void) {
  // initialize builtin functions ---------------------------------------------
  def_builtin_fun(&Main, "+", 2, false, OP_ADD);
  def_builtin_fun(&Main, "-", 2, false, OP_SUB);
  def_builtin_fun(&Main, "*", 2, false, OP_MUL);
  def_builtin_fun(&Main, "/", 2, false, OP_DIV);
  def_builtin_fun(&Main, "rem", 2, false, OP_REM);
  def_builtin_fun(&Main, "=", 2, false, OP_NEQ);
  def_builtin_fun(&Main, "<", 2, false, OP_NLT);
  def_builtin_fun(&Main, ">", 2, false, OP_NGT);
  def_builtin_fun(&Main, "=?", 2, false, OP_EGAL);
  def_builtin_fun(&Main, "hash", 1, false, OP_HASH);
  def_builtin_fun(&Main, "type", 1, false, OP_TYPE);
  def_builtin_fun(&Main, "list", 0, true, OP_LIST);
  def_builtin_fun(&Main, "cons", 2, false, OP_CONS);
  def_builtin_fun(&Main, "head", 1, false, OP_HEAD);
  def_builtin_fun(&Main, "tail", 1, false, OP_TAIL);
  def_builtin_fun(&Main, "list-ref", 2, false, OP_LIST_REF);
  def_builtin_fun(&Main, "list-len", 1, false, OP_LIST_LEN);
  def_builtin_fun(&Main, "str", 0, true, OP_STR);
  def_builtin_fun(&Main, "chars", 1, false, OP_CHARS);
  def_builtin_fun(&Main, "str-ref", 2, false, OP_STR_REF);
  def_builtin_fun(&Main, "str-len", 1, false, OP_STR_LEN);
  def_builtin_fun(&Main, "*heap-report*", 0, false, OP_HEAP_REPORT);
  def_builtin_fun(&Main, "*stack-report*", 0, false, OP_STACK_REPORT);
  def_builtin_fun(&Main, "*env-report*", 0, false, OP_ENV_REPORT);
  def_builtin_fun(&Main, "*dis*", 1, false, OP_DIS);
  def_builtin_fun(&Main, "load", 1, false, OP_LOAD);

  // initialize other globals -------------------------------------------------
  toplevel_env_def(Vm.globals, mk_sym(&Main, "&ins"), tag_obj(&Ins));
  toplevel_env_def(Vm.globals, mk_sym(&Main, "&outs"), tag_obj(&Outs));
  toplevel_env_def(Vm.globals, mk_sym(&Main, "&errs"), tag_obj(&Errs));
  toplevel_env_def(Vm.globals, mk_sym(&Main, "&globals"), tag_obj(Vm.globals));

  // special forms and other syntactic markers --------------------------------
  QuoteStr = mk_str(&Main, "quote");
  DefStr   = mk_str(&Main, "def");
  PutStr   = mk_str(&Main, "put");
  IfStr    = mk_str(&Main, "if");
  DoStr    = mk_str(&Main, "do");
  FnStr    = mk_str(&Main, "fn");
}

void initialize_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_TYPES; i++ ) {
    ExpTypeInfo* info = &Types[i]; strcpy(buffer+1, info->name);
    info->repr        = mk_sym(&Main, buffer);
  }
}

void initialize_standard_library(void) {
  // load the standard library file
  load_file(&Main, "lisp/boot.rl");
}

void print_welcome(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void setup(void) {
  init_standard_streams();
  init_static_objects();
  define_builtins();
  initialize_types();
  initialize_standard_library();
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
    repl(&Main);
  }

  teardown();

  return 0;
}
