#include "prin.h"

#include "type.h"

#include "util/ios.h"

/* API */
void prin(val_t x) {
  type_t type = type_of(x);

  if (type->prin)
    type->prin(x);

  else
    printf("<%s>", type->name);
}

void prinln(val_t x) {
  prin(x);
  newline();
}
