#include "util/io.h"


int fpeekc(FILE* ios) {
  int out = fgetc(ios);

  if (out != EOF)
    ungetc(out, ios);

  return out;
}

char* readFile(const char* path);
void  runFile(const char* path);
