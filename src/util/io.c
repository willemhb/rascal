#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util/io.h"

#include "memory.h"


int fpeekc(FILE* ios) {
  int out = fgetc(ios);

  if (out != EOF)
    ungetc(out, ios);

  return out;
}

char* readFile(const char* path) {
  FILE*  file   = SAFE_OPEN(path, "rb");
  size_t fSize  = fileSize(file);
  char*  buffer = SAFE_MALLOC(fSize+1);

  SAFE_READ(path, buffer, char, fSize, file);
  return buffer;
}

size_t fileSize(FILE* ios) {
  size_t pos = ftell(ios);
  rewind(ios);
  fseek(ios, 0, SEEK_END);
  size_t fend = ftell(ios);
  rewind(ios);
  fseek(ios, pos, SEEK_SET);
  return fend;
}
