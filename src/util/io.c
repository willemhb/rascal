#include "util/io.h"

// api ------------------------------------------------------------------------
int newln(void) {
  return printf("\n");
}

int fnewln(FILE* ios) {
  return fprintf(ios, "\n");
}

int peekc(void) {
  return fpeekc(stdin);
}

int fpeekc(FILE* ios) {
  int out = fgetc(ios);

  if (out != EOF)
    ungetc(out, ios);

  return out;
}

void cleanf(FILE** ios) {
  if (ios && *ios != NULL)
    fclose(*ios);
}
