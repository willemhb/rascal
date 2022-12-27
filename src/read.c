#include "read.h"
#include "reader.h"

#include "util/ios.h"

/* API */
val_t read(void) {
  return read_expression(&Reader);
}

val_t readln(void) {
  val_t out = read();

  newline();

  return out;
}
