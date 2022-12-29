#include "repl.h"

#include "read.h"
#include "eval.h"
#include "prin.h"

#include "type.h"

#include "util/ios.h"

/* globals */
#define PROMPT "rascal>"

/* API */
void repl(void) {
  val_t x, v;

  for (;;) {
    printf(PROMPT" ");
    x = readln();
    v = eval(x);

#ifdef DEBUG
    printf("%s> ", type_of(v)->name);
#endif

    prinlns(v, 2);
  }
}
