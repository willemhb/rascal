#include "read.h"

#include "list.h"

#include "reader.h"
#include "error.h"

#include "util/ios.h"

/* API */
val_t read(void) {
  val_t out = read_expression(&Reader);

  if (recover()) {
    sync_reader(&Reader, '\n');
    return NUL;
  }

  return out;
}

val_t readln(void) {
  extern int peekchr(reader_t *reader);
  
  val_t out = read();

  int ch;
  
  if ((ch=peekchr(&Reader)) != '\n') {
    printf("\nUnexpected character '%c' reading line.\n\n", ch);
    sync_reader(&Reader, '\n');
    return NUL;
  }

  newline();

  return out;
}
