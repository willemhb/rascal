#include <stdlib.h>
#include <stdio.h>

#include "util/hashing.h"
#include "util/io.h"

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
    .annot =NULL,
    .hash  =0,
    .type  =LIST,
    .hashed=false,
    .flags =0,
    .black =false,
    .gray  =true
  },
  .tail =&emptyList,
  .arity=0,
  .head =NUL
};

Vector emptyVector = {
  .obj={
    .next  =NULL,
    .annot =NULL,
    .hash  =0,
    .type  =VECTOR,
    .hashed=false,
    .flags =0,
    .black =false,
    .gray  =true
  },
  .arity=0,
  
};

Map emptyMap = {
  
};

// special forms and other important symbols
Value FunSym, VarSym, IfSym, WithSym, QuoteSym, DoSym, UseSym;

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
  extern size_t compileFun(Vm* vm, List* form);
  extern size_t compileVar(Vm* vm, List* form);
  extern size_t compileIf(Vm* vm, List* form);
  extern size_t compileWith(Vm* vm, List* form);
  extern size_t compileQuote(Vm* vm, List* form);
  extern size_t compileDo(Vm* vm, List* form);
  extern size_t compileUse(Vm* vm, List* form);

  FunSym   = defineSpecial("fun", compileFun);
  VarSym   = defineSpecial("var", compileVar);
  IfSym    = defineSpecial("if", compileIf);
  WithSym  = defineSpecial("with", compileWith);
  QuoteSym = defineSpecial("quote", compileQuote);
  DoSym    = defineSpecial("do", compileDo);
  UseSym   = defineSpecial("use", compileUse);

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
