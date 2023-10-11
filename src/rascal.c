#include <stdlib.h>
#include <stdio.h>

#include "util/hashing.h"
#include "util/io.h"

#include "debug.h"
#include "vm.h"
#include "interpreter.h"

// version information
#define MAJOR 0
#define MINOR 0
#define PATCH 4
#define DEV   "a"
#define VFMT "%.2d.%.2d.%.2d.%s"


// majority of global state lives in vm
Vm vm;

// global stack space
Value TheStack[N_STACK];

// empty singleton objects
List emptyList = {
  .obj={
    .next  =NULL,
    .hash  =0,
    .type  =LIST,
    .hashed=false,
    .flags =0,
    .black =false,
    .gray  =true
  },
  .tail =&emptyList,
  .arity=0,
  .head =NUL_VAL
};

// special forms and other important symbols
Value QuoteSym, DoSym, VarSym, IfSym;

// startup helpers & shutdown
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
  // initialize hashes for global singletons
  emptyList.obj.hash   = hashPtr(&emptyList);
  emptyList.obj.hashed = true;

  // initialize virtual machine (contains most global state)
  initVm(&vm);

  // initialize special forms
  extern size_t compileQuote(Compiler* compiler, List* form);
  extern size_t compileDo(Compiler* compiler, List* form);
  extern size_t compileVar(Compiler* compiler, List* form);
  extern size_t compileIf(Compiler* compiler, List* form);

  QuoteSym = defineSpecial("quote", compileQuote);
  DoSym    = defineSpecial("do", compileDo);
  VarSym   = defineSpecial("var", compileVar);
  IfSym    = defineSpecial("if", compileIf);

  // print welcome
  welcomeMessage();
}

static void finalizeRascal(void) {
  freeVm(&vm);
  goodbyeMessage();
}

int main(const int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  initRascal();
  repl(&vm);
  finalizeRascal();

  return 0;
}
