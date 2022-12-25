#include "read.h"
#include "eval.h"
#include "prin.h"
#include "comp.h"

#include "sym.h"
#include "list.h"

#include "util/ios.h"


/* globals */
#define PROMPT "rascal>"


/* API */
static inline bool is_lit(val_t x) {
  if (is_sym(x))
    return *as_sym(x) == ':';

  return !is_cons(x);
}

val_t eval(val_t x) {
  if (is_lit(x))
    return x;

  if (is_sym(x))
    return sym_val(x);

  module_t m = comp(x);
  return exec(m);
}

void repl(void) {
  for (;;) {
    printf(PROMPT" ");
    val_t x = read();
    newline();
    prin(x);
    newline();
  }
}

/* initialization */
void eval_init(void) {}
