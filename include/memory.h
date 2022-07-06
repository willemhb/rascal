#ifndef rascal_memory_h
#define rascal_memory_h

#include "core.h"

// flags stored for each 
typedef enum {
  mem_blackbit  = 0x01,
  mem_allocated = 0x02,
  mem_relocated = 0x04,
  mem_obhead    = 0x08,
  mem_obtail    = 0x10,
  mem_traced    = 0x20,
  mem_printed   = 0x40,
  mem_finalize  = 0x80
} memflags_t;

// exports --------------------------------------------------------------------
bool_t   globalp( void_t *ptr );
uchar_t *findmap( void_t *ptr );
bool_t   overflowp( size_t n );
uchar_t getmemfl( void_t *ptr );
uchar_t setmemfl( void_t *ptr, uchar_t fl );
uchar_t clearmemfl( void_t *ptr, uchar_t fl );

void_t   *allocate( size_t nbytes, bool_t global );
object_t *construct( type_t type, size_t extra, bool_t global );

void    manage( void ); // GC entry point

// initialization -------------------------------------------------------------
void    initmemory( void );

// utilities ------------------------------------------------------------------
#define movedp(x) ( obtag(x)==tag_moved )
#define getfp(x)  ( (void_t*)(obhead(x)&~2ul) )

#endif
