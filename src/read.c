#include "read.h"
#include "reader.h"

/* API */
val_t read(void) {
  return read_expression(&Reader);
}
