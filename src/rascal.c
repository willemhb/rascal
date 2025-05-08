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

// setup ----------------------------------------------------------------------
void init_standard_streams(void) {
  // add FILE* objects to corresponding standard ports
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;
}

void init_static_objects(void) {
  // register static global objects in heap
  // so that they get unmarked during the sweep phase
  add_to_heap(&Globals);
  add_to_heap(&Ins);
  add_to_heap(&Outs);
  add_to_heap(&Errs);
}

void define_builtins(void) {
  // initialize builtin functions
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
  def_builtin_fun("*dis*", OP_DIS);

  // initialize other globals
  toplevel_env_def(&Globals, mk_sym("&ins"), tag_obj(&Ins));
  toplevel_env_def(&Globals, mk_sym("&outs"), tag_obj(&Outs));
  toplevel_env_def(&Globals, mk_sym("&errs"), tag_obj(&Errs));
  
  // special forms and other syntactic markers
  QuoteStr = mk_str("quote");
  DefStr   = mk_str("def");
  PutStr   = mk_str("put");
  IfStr    = mk_str("if");
  DoStr    = mk_str("do");
  FnStr    = mk_str("fn");
  CatchStr = mk_str("catch");
  ThrowStr = mk_str("throw");
}

void init_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_TYPES; i++ ) {
    ExpTypeInfo* info = &Types[i]; strcpy(buffer+1, info->name);
    info->repr        = mk_sym(buffer);
  }
}

void init_etypes(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_ERRORS; i++ ) {
    strcpy(buffer+1, ErrorNames[i]);

    ErrorTypes[i] = mk_sym(buffer);
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
  init_types();
  init_etypes();
  print_welcome();
}

// teardown -------------------------------------------------------------------
void teardown(void) {}

// rascal! --------------------------------------------------------------------
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  setup();
  repl();
  teardown();

  return 0;
}
