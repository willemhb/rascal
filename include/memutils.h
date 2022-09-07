#ifndef memutils_h
#define memutils_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void *malloc_s( size_t nbytes );
void *realloc_s( void *p, size_t nbytes );
void *calloc_s( size_t cnt, size_t obsize );
void  free_s( void *p );

size_t  aligned( size_t nbytes, size_t alignment );
bool    _within_s( void *p, void *base, size_t size );
bool    _within_p( void *p, void *base, void *n );
bool    _within_i( void *p, void *base, int size );

#define withinp(_p, _b, _a)			\
  _Generic((_a),				\
	   size_t:_within_s,			\
	   int:_within_i,			\
	   default:_within_p)((_p),(_b),(_a))

#endif
