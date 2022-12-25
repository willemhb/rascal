#include "num.h"
#include "prin.h"

/* API */
/* internal */
void prin_real(val_t x) {
  printf("%.2f", as_real(x));
}

/* initialization */
void num_init(void) {
  Prin[real_val]     = prin_real;
  TypeName[real_val] = "real";
}
