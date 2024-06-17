#include "lang/read.h"

#include "val/text.h"

#include "vm/reader.h"

/* Forward declarations */
/* Miscellaneous readers */
void rl_read_eof(RState* s, int d);
void rl_read_fail(RState* s, int d);

/* Ignore/separator readers */
void rl_read_comment(RState* s, int d);
void rl_read_space(RState* s, int d);

/* Macro readers */
void rl_read_quote(RState* s, int d);
void rl_read_hash(RState* s, int d);
void rl_read_tick(RState* s, int d);

/* Collection readers */
void rl_read_str(RState* s, int d);
void rl_read_bin(RState* s, int d);
void rl_read_list(RState* s, int d);
void rl_read_vec(RState* s, int d);
void rl_read_map(RState* s, int d);

/* Atomic readers */
void rl_read_glyph(RState* s, int d);
void rl_read_num(RState* s, int d);
void rl_read_sym(RState* s, int d);


/* Globals */
#define DIGITS "012345678"
#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define PUNCT  "+-*/=_:?^"
#define SPACE  " ,\t\n\r"

/* Initialization */
void init_toplevel_rt(void) {
  init_rt(&Rt, NULL);

  /* Add miscellaneous readers readers */
  rt_set(&Rt, EOF,    rl_read_eof,     false);

  /* Add ignore/separator readers */
  rt_set(&Rt, ';',    rl_read_comment, false);
  rt_set(&Rt, SPACE,  rl_read_space,   false);

  /* Add macro readers */
  rt_set(&Rt, '\'',   rl_read_quote,   false);
  rt_set(&Rt, '#',    rl_read_hash,    false);
  rt_set(&Rt, '`',    rl_read_tick,    false);

  /* Add collection readers */
  rt_set(&Rt, '"',    rl_read_str,     false);
  rt_set(&Rt, '<',    rl_read_bin,     false);
  rt_set(&Rt, '(',    rl_read_list,    false);
  rt_set(&Rt, '[',    rl_read_vec,     false);
  rt_set(&Rt, '{',    rl_read_map,     false);

  /* Add atomic readers */
  rt_set(&Rt, '\\',   rl_read_glyph, false);
  rt_set(&Rt, DIGITS, rl_read_num,   false);
  rt_set(&Rt, UPPER,  rl_read_sym,   false);
  rt_set(&Rt, LOWER,  rl_read_sym,   false);
  rt_set(&Rt, PUNCT,  rl_read_sym,   false);
}
