#include <stdlib.h>
#include <stdio.h>

#include "util/hashing.h"
#include "util/io.h"

#include "runtime.h"

#include "type.h"
#include "function.h"
#include "environment.h"
#include "collection.h"
#include "stream.h"

#include "read.h"
#include "compile.h"
#include "eval.h"

// version information
#define M_V 0
#define m_V 0
#define P_V 4
#define D_V   "a"
#define VFMT "%.2d.%.2d.%.2d.%s"

// startup helpers & shutdown
static void welcome_message(void) {
  fprintf(stdout, "Welcome to Rascal version "VFMT"!\n", M_V, m_V, P_V, D_V);
}

static void goodbye_message(void) {
  fprintf(stdout, "Goodbye!\n");
}

static void init_cli_args(const int argc, const char* argv[]) {
  Value buf[argc];

  for (int i=0; i<=argc; i++) {
    char* s  = (char*)argv[i];
    size_t l = strlen(s);

    buf[i] = tag(new_str(s, l, ASCII));
  }

  List* args = new_list(argc, buf);

  define(NULL, symbol("&args"), tag(args), CONSTANT);
}

static void init_rascal(const int argc, const char* argv[]) {
  // initialize hashes for global singletons
  init_global_singletons();
  init_vm(&RlVm);
  init_builtin_types();
  init_options();
  init_std_streams();
  init_builtin_functions();
  init_builtin_readers();
  init_special_forms();
  init_cli_args(argc, argv);

  RlVm.m.init = true;
  // print welcome
  welcome_message();
}

static void finalize_rascal(void) {
  free_vm(&RlVm);
  goodbye_message();
}

int main(const int argc, const char* argv[]) {
  init_rascal(argc, argv);
  repl();
  finalize_rascal();
  
  return 0;
}
