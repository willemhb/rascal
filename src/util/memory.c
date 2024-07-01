#include <stdlib.h>
#include <string.h>

#include "util/memory.h"


/* external API */
void* rl_malloc(size_t n, byte i) {
  void* b = NULL;
  
  if ( n > 0 ) {
    b = malloc(n);

    if ( unlikely(b == NULL) )
      rl_fatal_err(SYSTEM_ERROR, "malloc", "couldn't allocate %zu bytes", n);
    
    memset(b, i, n);
  }
  
  return b;
}

void* rl_calloc(size_t n, size_t o, byte i) {
  void* b = NULL;

  if ( n > 0 && o > 0 ) {
    b = calloc(n, o);

    if ( unlikely(b == NULL) )
      rl_fatal_err(SYSTEM_ERROR, "calloc", "couldn't allocate %zu bytes", n*o);

    memset(b, i, n*o);
  }

  return b;
}

void* rl_mdup(void* s, size_t n) {
  void* b = NULL;

  if ( s != NULL ) {
    b = rl_malloc(n, 0);

    memcpy(b, s, n);
  }

  return b;
}

char* rl_sdup(char* s, size_t n) {
  char* b = NULL;

  if ( s != NULL ) {
    b = rl_malloc(n+1, 0);

    memcpy(b, s, n);
  }

  return b;
}

void* rl_realloc(void* s, size_t p, size_t n, byte i) {
  void* o = NULL;

  if ( n == 0 ) {
    free(s);

  } else {
    o = realloc(s, n);

    if ( unlikely(o == NULL) )
      rl_fatal_err(SYSTEM_ERROR, "realloc", "couldn't allocate %zu bytes", n);

    if ( n > p )
      memset(o+p, i, n-p);
  }

  return o;
}

void* rl_crealloc(void* s, size_t p, size_t n, size_t o, byte i) {
  void* b = NULL;

  if ( n == 0 || o == 0 ) {
    free(s);
  } else {
    b = realloc(s, n*o);

    if ( unlikely(b == NULL) )
      rl_fatal_err(SYSTEM_ERROR, "realloc", "couldn't allocate %zu bytes", n*o);

    if ( n > p )
      memset(b+p*o, i, (n-p)*o);
  }

  return b;
}

void rl_free(void* b) {
  free(b);
}
