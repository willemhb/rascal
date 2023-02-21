#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "util/string.h"
#include "util/memory.h"
#include "util/io.h"

// generic memory comparison --------------------------------------------------
int u8cmp(uint8* xs, uint8* ys, usize n) {
  return memcmp(xs, ys, n);
}

int u16cmp(uint16* xs, uint16* ys, usize n) {
  int o;
  
  for (;n;xs++, ys++, n--)
    if ((o=*xs-*ys))
      break;

  return o;
}

int  u32cmp(uint32* xs, uint32* ys, usize n) {
  int o;

  for (;n;xs++,ys++,n--)
    if ((o=*xs-*ys))
      break;

  return o;
}

long u64cmp(uint64* xs, uint64* ys, usize n) {
  long o;

  for (;n;xs++,ys++,n--)
    if ((o=*xs-*ys))
      break;

  return o;
}

// create a string of indeterminate size --------------------------------------
char* strfmt(char* fmt, ...) {
  va_list va;
  va_start(va, fmt);

  char* out = vstrfmt(fmt, va);

  va_end(va);

  return out;
}

char* vstrfmt(char* fmt, va_list va) {
  FILE* tmp cleanup(cleanf) = SAFE_OPEN(tmpfile);
  
  int n = vfprintf(tmp, fmt, va);

  assert(n >= 0);

  rewind(tmp);

  char* buf = calloc_s(n+1, sizeof(char));

  fread(buf, sizeof(char), n, tmp);

  buf[n] = '\0';

  return buf;
}

