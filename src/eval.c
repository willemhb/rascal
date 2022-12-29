#include "eval.h"

#include "comp.h"
#include "exec.h"

#include "sym.h"
#include "list.h"
#include "module.h"

#include "type.h"
#include "vm.h"

#include "def/opcodes.h"

#include "util/ios.h"

/* globals */

/* API */
bool is_lit(val_t x) {
  if (is_sym(x))
    return *as_sym(x) == ':';

  return !is_cons(x);
}

val_t eval(val_t x) {
  if (is_lit(x))
    return x;

  if (is_sym(x))
    return sym_head(x)->val;

  module_t m = comp(x);
  return exec(m);
}
