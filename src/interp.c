#include "interp.h"
#include "reader.h"
#include "val.h"
#include "obj.h"

#include "util/ios.h"


/* globals */
void prin_list(val_t x);
void prin_sym(val_t x);
void prin_real(val_t x);

void (*Prin[num_types])(val_t x) = {
  [real_type] = prin_real,
  [sym_type]  = prin_sym,
  [nul_type]  = prin_list,
  [cons_type] = prin_list
};

char *TypeName[num_types] = {
  [nul_type]  = "nul",
  [real_type] = "real",
  [sym_type]  = "sym",
  [cons_type] = "cons"
};

/* API */
/* interpreter core */
val_t read(void) {
  return read_expression(&Reader);
}

val_t eval(val_t x) {
  return x;
}

void prin(val_t x) {
  rl_type_t type = rl_type(x);

  if (Prin[type])
    Prin[type](x);

  else
    printf("<%s>", TypeName[type]);
}

/* repl */
#define PROMPT "rascal>"

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
void interp_init( void ) {}
