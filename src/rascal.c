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
  add_to_heap(&Globals);
  add_to_heap(&Ins);
  add_to_heap(&Outs);
  add_to_heap(&Errs);
}

void define_builtins(void) {
  // initialize builtin functions ---------------------------------------------
  def_builtin_fun("+", OP_ADD);
  def_builtin_fun("-", OP_SUB);
  def_builtin_fun("*", OP_MUL);
  def_builtin_fun("/", OP_DIV);
  def_builtin_fun("=", OP_EGAL);
  def_builtin_fun("type", OP_TYPE);
  def_builtin_fun("cons", OP_CONS);
  def_builtin_fun("head", OP_HEAD);
  def_builtin_fun("tail", OP_TAIL);
  def_builtin_fun("nth", OP_NTH);
  def_builtin_fun("*heap-report*", OP_HEAP_REPORT);
  def_builtin_fun("load", OP_LOAD);

  // initialize other globals -------------------------------------------------
  toplevel_env_def(&Globals, mk_sym("&ins"), tag_obj(&Ins));
  toplevel_env_def(&Globals, mk_sym("&outs"), tag_obj(&Outs));
  toplevel_env_def(&Globals, mk_sym("&errs"), tag_obj(&Errs));
  
  // special forms and other syntactic markers --------------------------------
  QuoteStr = mk_str("quote");
  DefStr   = mk_str("def");
  PutStr   = mk_str("put");
  IfStr    = mk_str("if");
  DoStr    = mk_str("do");
  FnStr    = mk_str("fn");
}

void initialize_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_TYPES; i++ ) {
    ExpTypeInfo* info = &Types[i]; strcpy(buffer+1, info->name);
    info->repr        = mk_sym(buffer);
  }
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
    load_file(argv[optind]);
  } else {
    // Enter REPL
    print_welcome();
    repl();
  }

  teardown();

  return 0;
}
