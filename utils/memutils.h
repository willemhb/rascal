#ifndef rascal_memutils_h
#define rascal_memutils_h

#include <stddef.h>
#include <stdbool.h>

// safe dynamic allocation ----------------------------------------------------
void *malloc_s( size_t n_bytes );
void *realloc_s( void *p, size_t n_bytes );
void *calloc_s( size_t cnt, size_t ob_size );
void  free_s( void *p );

// helpers --------------------------------------------------------------------
size_t aligned( size_t n_bytes, size_t alignment );

bool _within_s( void *p, void *base, size_t n_bytes );
bool _within_p( void *p, void *base, void *end );

#define within(p, b, n)				\
  _Generic((n),					\
	   size_t: _within_s,			\
	   default: _within_p			\
	   )(p, b, n)

#endif
