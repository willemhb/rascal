#include <errno.h>

#include "util/io.h"

#include "memory.h"


int fpeekc(FILE* ios) {
  int out = fgetc(ios);

  if (out != EOF)
    ungetc(out, ios);

  return out;
}

char* read_path(const char* path) {
  FILE*  file   = SAFE_OPEN(path, "rb");
  return read_file(path, file);
}

char* readFile(const char* path, FILE* ios) {
  size_t f_size = file_size(ios);
  char*  buffer = SAFE_MALLOC(f_size+1);
  
  SAFE_READ(path, buffer, char, f_size, ios);
  fclose(ios);
  return buffer;
}

size_t file_size(FILE* ios) {
  size_t pos = ftell(ios);
  rewind(ios);
  fseek(ios, 0, SEEK_END);
  size_t fend = ftell(ios);
  rewind(ios);
  fseek(ios, pos, SEEK_SET);
  return fend;
}
