#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "vm.h"
#include "debug.h"
#include "reader.h"
#include "interpreter.h"


Value eval(Vm* vm, Value expr) {
  
  return expr;
}


// external API
void initInterpreter(Interpreter* interpreter, Chunk* code) {
  interpreter->sp   = TheStack;
  interpreter->code = code;

  if (code != NULL)
    interpreter->ip = code->code.data;
}

void freeInterpreter(Interpreter* interpreter) {
  interpreter->sp   = TheStack;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void repl(Vm* vm) {
  #define REPL_BUFFER_SIZE 8192

  static const char*  prompt  = "rascal>";

  for (;;) {
    fprintf(stdout, "%s ", prompt);
    Value xpr = read(&vm->reader);
    fprintf(stdout, "\n");
    Value val = eval(vm, xpr);
    printValue(stdout, val);
    fprintf(stdout, "\n");
  }
}
