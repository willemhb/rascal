#include <ctype.h>
#include <string.h>

#include "util/ios.h"

/* API */
int fpeekc(FILE *file) {
  if (feof(file))
    return EOF;

  int out = fgetc(file);

  ungetc(out, file);

  return out;
}

bool isrlspc(int ch) {
  return strchr(RLSPACES, ch);
}

bool isrlsymchr(int ch) {
  static const char* nonsym = "[](){},;'`~@";

  if (isspace(ch) || iscntrl(ch))
    return false;

  return !strchr(nonsym, ch);
}

bool isrldlm(int ch) {
  return strchr(RLSPACES""PAIRED, ch);
}

int newline(void) {
  return printf("\n");
}

int fnewline(FILE *file) {
  return fprintf(file, "\n");
}
