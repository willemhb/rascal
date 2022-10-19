#ifndef rascal_memory_h
#define rascal_memory_h

#include "obj/memory.h"

// forward declarations, macros, and globals for the memory runtime

// globals
extern heap_t Heap;

// florward declarations
void     *alloc(size_t n);
object_t *allocob(size_t n);
void     *resize(void *p, size_t o, size_t n);
void      dealloc(void *p, size_t n);
void      collect_garbage(void);
void     *duplicate_bytes(void *p, size_t n);
char     *duplicate_string(char *chars);

#define safe_alloc(allcfun, ...)		\
  ({						\
    void *out = allcfun( __VA_ARGS__ );		\
    if (out == NULL)				\
      {						\
	printf("error at %s:%d:%s: OOM.",	\
	       __FILE__,			\
	       __LINE__,			\
	       __func__);			\
	exit(1);				\
      }						\
    out;					\
  })

#define malloc_s(n)        safe_alloc(malloc, (n))
#define calloc_s(n, s)     safe_alloc(calloc, (n), (s))
#define realloc_s(p, n)    safe_alloc(realloc, (p), (n))

#define duplicate(p, ...)						\
  _Generic((p),								\
	   char*:duplicate_string,					\
	   default:duplicate_bytes)((p) __VA_OPT__(,) __VA_ARGS__)

// initialization
void heap_init( void );

#endif
