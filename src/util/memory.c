#include <stdlib.h>
#include <string.h>

#include "vm/error.h"

#include "util/memory.h"
#include "util/number.h"

/* external API */
static char* aerr_fmt = "couldn't allocate %zu bytes";

#define alloc_err(f, n) rl_sys_err(&Main, f, aerr_fmt, n)

void* s_malloc(char* f, size64 n, byte i) {
  void* b = NULL;
  
  if ( n > 0 ) {
    b = calloc(n, 1);

    if ( unlikely(b == NULL) )
      alloc_err(f, n);

    if ( i != 0 )
      memset(b, i, n);
  }
  
  return b;
}

void* s_calloc(char* f, size64 n, size64 o, byte i) {
  void* b = NULL;

  if ( n > 0 && o > 0 ) {
    b = calloc(n, o);

    if ( unlikely(b == NULL) )
      alloc_err(f, n*o);

    if ( i != 0 )
      memset(b, i, n*o);
  }

  return b;
}

void* s_salloc(char* f, size64 n, size64 o, byte i, bool ae) {
  void* b = NULL;

  if ( (n > 0 || ae) && o > 0 ) {
    b = calloc(n, o);

    if ( unlikely(b == NULL) )
      alloc_err(f, (n+1)*o);

    if ( i != 0 )
      memset(b, i, n*o);
  }

  return b;
}

void* s_mdup(char* f, void* s, size64 n) {
  void* b = NULL;

  if ( s != NULL ) {
    b = s_malloc(f, n, 0);

    memcpy(b, s, n);
  }

  return b;
}

char* s_sdup(char* f, char* s, size64 n) {
  char* b = NULL;

  if ( s != NULL ) {
    b = s_malloc(f, n+1, 0);

    memcpy(b, s, n);
  }

  return b;
}

void* s_realloc(char* f, void* s, size64 p, size64 n, byte i) {
  void* o = NULL;

  if ( n == 0 ) {
    free(s);

  } else {
    o = realloc(s, n);

    if ( unlikely(o == NULL) )
      alloc_err(f, n);

    if ( n > p )
      memset(o+p, i, n-p);
  }

  return o;
}

void* s_crealloc(char* f, void* s, size64 p, size64 n, size64 o, byte i) {
  void* b = NULL;

  if ( n == 0 || o == 0 ) {
    s_free(f, s);
  } else {
    b = realloc(s, n*o);

    if ( unlikely(b == NULL) )
      alloc_err(f, n*o);

    if ( n > p )
      memset(b+p*o, i, (n-p)*o);
  }

  return b;
}

void* s_srealloc(char* f, void* s, size64 p, size64 n, size64 o, byte i, bool ae) {
  (void)i;

  void* b = NULL;

  if ( (n == 0 && !ae) || o == 0 ) {
    s_free(f, s);
  } else {
    b = calloc(n+1, o);

    if ( unlikely(b == NULL) )
      alloc_err(f, (n+1)*o);

    size64 nc = min(n, p);

    memcpy(b, s, nc);
    s_free(f, s);
  }

  return b;
}

void s_free(char* f, void* b) {
  (void)f;
  free(b);
}

#undef alloc_err
