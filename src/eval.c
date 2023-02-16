#include "eval.h"
#include "read.h"
#include "print.h"
#include "exec.h"
#include "compile.h"
#include "object.h"
#include "utils.h"

/* globals */
#define PROMPT ">> "

/* API */
Val eval(Val x) {
  if (is_nonempty_list(x)) {
    UserFn *compiled = compile(x);
    return exec(compiled);
  } else if (is_variable_sym(x)) {
    return lookup(x);
  } else {
    return x;
  }
}

void repl(void) {
  for (;;) {
    printf(PROMPT);
    Val x = read();
    Val v = eval(x);
    newln();
    print(v);
    newln();
  }
}
