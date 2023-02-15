#include "read.h"
#include "reader.h"

/* API */
Val read_expr(struct Reader *reader);

Val read(void) {
  Val out = read_expr(&Reader);

  return out;
}

