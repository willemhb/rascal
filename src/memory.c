#include <stdlib.h>
#include <string.h>

#include "memory.h"


/* external API */
rl_status_t rl_malloc(void** b, size_t n, byte_t i) {
  rl_status_t r = OKAY;
  
  if ( n == 0 ) {
    *b = NULL;

  } else {
    void* spc = malloc(n);

    if ( unlikely(spc == NULL) )
      rl_fatal_error(SYSTEM_ERROR, "malloc", "couldn't allocate %zu bytes", n);

    memset(spc, i, n);

    *b = spc;
  }

  return r;
}

rl_status_t rl_calloc(void** b, size_t n, size_t o, byte_t i) {
  rl_status_t r = OKAY;

  if ( n == 0 || o == 0 ) {
    *b = NULL;
    
  } else {
    void* spc = calloc(n, o);

    if ( unlikely(spc == NULL) )
      rl_fatal_error(SYSTEM_ERROR, "calloc", "couldn't allocate %zu bytes", n*o);

    memset(spc, i, n*o);
  }

  return r;
}

rl_status_t rl_realloc(void** b, size_t p, size_t n, byte_t i) {
  rl_status_t r = OKAY;
  void* spc = *b;

  if ( n == 0 ) {
    free(spc);
    *b = NULL;

  } else {
    spc = realloc(spc, n);

    if ( unlikely(spc == NULL) )
      rl_fatal_error(SYSTEM_ERROR, "realloc", "couldn't allocate %zu bytes", n);

    if ( n > p )
      memset(spc+p, i, n-p);

    *b = spc;
  }

  return r;
}

rl_status_t rl_crealloc(void** b, size_t p, size_t n, size_t o, byte_t i) {
  rl_status_t r = OKAY;
  void* spc = b;

  if ( n == 0 || o == 0 ) {
    free(spc);
    *b = NULL;
  } else {
    spc = realloc(spc, n*o);

    if ( unlikely(spc == NULL) )
      rl_fatal_error(SYSTEM_ERROR, "realloc", "couldn't allocate %zu bytes", n*o);

    if ( n > p )
      memset(spc+p*o, i, (n-p)*o);

    *b = spc;
  }

  return r;
}

rl_status_t rl_free(void* b) {
  rl_status_t r = OKAY;

  free(b);

  return r;
}
