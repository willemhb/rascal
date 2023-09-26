#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "vm.h"
#include "debug.h"
#include "interpreter.h"

Value runFile(Vm* vm, const char* fname) {
  char* source = readFile(fname);
  scan(&vm->scanner, source);
  Value result = parse(&vm->parser, &vm->scanner);
  freeParser(&vm->parser);
  freeScanner(&vm->scanner);
  free(source);
  return result;
}

Value eval(Vm* vm, char* source) {
  scan(&vm->scanner, source);
  Value out = parse(&vm->parser, &vm->scanner);
  return out;
}

void repl(Vm* vm) {
  #define REPL_BUFFER_SIZE 8192

  static const char*  prompt  = "rascal>";

  char   buffer[REPL_BUFFER_SIZE];

  for (;;) {
    fprintf(stdout, "%s ", prompt);
    char* source = fgets(buffer, REPL_BUFFER_SIZE, stdin);

    if (source == NULL) {
      fprintf(stderr, "fgets() failed for no obvious reason.\n");
      exit(74);
    }

    fprintf(stdout, "\n");
    Value xpr = eval(vm, buffer);
    if (xpr != NOTHING_VAL)
      printValue(stdout, xpr, -1);
    fprintf(stdout, "\n");
    freeScanner(&vm->scanner);
    freeParser(&vm->parser);
  }
}
