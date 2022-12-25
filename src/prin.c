#include "prin.h"
#include "val.h"

#include "util/ios.h"

/* globals */
void (*Prin[num_types])(val_t x) = {};
char  *TypeName[num_types] = {};

/* API */
void prin(val_t x) {
  rl_type_t type = rl_type(x);

  if (Prin[type])
    Prin[type](x);

  else
    printf("<%s>", TypeName[type]);
}
