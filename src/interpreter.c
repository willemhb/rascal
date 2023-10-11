#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "vm.h"
#include "debug.h"
#include "reader.h"
#include "interpreter.h"

// external API
void initInterpreter(Interpreter* interpreter, Value* vals, size_t nStack) {
  interpreter->sp   = vals;
  interpreter->vp   = vals;
  interpreter->ep   = vals+nStack;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void freeInterpreter(Interpreter* interpreter) {
  interpreter->sp   = NULL;
  interpreter->vp   = NULL;
  interpreter->ep   = NULL;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void resetInterpreter(Interpreter* interpreter, Chunk* code) {
  interpreter->sp   = interpreter->vp;
  interpreter->code = code;
  interpreter->ip   = code->code.data;
}

