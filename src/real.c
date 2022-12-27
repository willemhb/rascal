#include "real.h"

#include "type.h"

/* globals */
void prin_real(val_t val);

struct type_t RealType = {
  .name="real",
  .prin=prin_real
};

/* API */
/* internal */
void prin_real(val_t val) {
  printf("%.2g", as_real(val));
}
