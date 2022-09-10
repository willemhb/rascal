#include "memutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// safe malloc substitutes ----------------------------------------------------
void *malloc_s( size_t n_bytes )
{
  void *out = malloc( n_bytes );

  if ( out == NULL )
    {
      fprintf( stderr, "Out of memory, exiting.\n" );
      exit( 1 );
    }

  memset( out, 0, n_bytes );
  return out;
}

void *realloc_s( void* p, size_t nbytes ) {
  void *out = realloc( p, nbytes );

  if ( out == NULL ) {
      fprintf( stderr, "Out of memory, exiting.\n" );
      exit( 1 );
    }

  return out;
}

void *calloc_s( size_t cnt, size_t obsize ) {
  void *out = calloc( cnt, obsize );

  if ( out == NULL ) {
    fprintf( stderr, "Out of memory, exiting.\n" );
    exit( 1 );
  }

  memset( out, 0, cnt*obsize );
  return out;
}

void free_s( void *p ) {
  free( p );
}

// helpers --------------------------------------------------------------------
size_t aligned( size_t nbytes, size_t alignment ) {
  if ( nbytes < alignment )
    return alignment;

  size_t mod     = nbytes % alignment;
  bool   divides = mod == 0;

  return nbytes + ((alignment - mod) * !divides);
}

inline bool _within_s( void *p, void *base, size_t nbytes ) {
  return p >= base && (uint8_t*)p < ((uint8_t*)base + nbytes);
}

inline bool _within_p( void *p, void *base, void *end ) {
  return p >= base && p < end;
}
