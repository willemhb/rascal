#include "bool.h"

#include "type.h"

/* globals */
void prin_bool(val_t x);

struct type_t BoolType = {
  .name="bool",
  .prin=prin_bool
};

/* API */
/* external */
bool Cbool(val_t x) {
  return x != FALSE && x != OBJECT;
}

/* internal */
void prin_bool(val_t x) {
  if (x == TRUE)
    printf("true");

  else
    printf("false");
}

/* native functions */
#include "func.h"
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

val_t native_bool(size_t nargs, val_t *args) {
  (void)nargs;

  if (Cbool(args[0]))
    return TRUE;

  return FALSE;
}

val_t native_not(size_t nargs, val_t *args) {
  (void)nargs;

  if (Cbool(args[0]))
    return FALSE;

  return TRUE;
}

void bool_init(void) {
  val_t native_bool_val = native("bool", 1, false, NULL, &BoolType, native_bool);
  val_t native_not_val  = native("not", 1, false, NULL, NULL, native_not);

  define("bool", native_bool_val);
  define("not", native_not_val);
}
