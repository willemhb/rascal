#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

enum memfl_t {
  // copies of the common object flags
  memfl_printed  = 0x0001,
  memfl_traced   = 0x0002,
  memfl_protect  = 0x0004,
  memfl_finalize = 0x0008,

  // flags stored only in the heap bitmap
  mem_allocated  = 0x0010,
  mem_relocated  = 0x0020,
  mem_unboxed    = 0x0040,
  mem_blackbit   = 0x0080,

  // flags used for allocation
  memfl_shared   = 0x0100,
  memfl_global   = 0x8000
};

// exports --------------------------------------------------------------------
void    *getmap( void *p );
void    *getword( void *p );
size_t   getoff( void *p );
bool_t   overflowp( size_t n );

uchar_t  getmemfl( void_t *ptr );
uchar_t  setmemfl( void_t *ptr, uchar_t fl );
uchar_t  clearmemfl( void_t *ptr, uchar_t fl );

// allocation procedures ------------------------------------------------------
void     *allocate( flags_t fl , size_t n );
object_t *reallocate( object_t *ob, size_t ol, size_t nl, void **p );

// GC entry point
void      manage( void );

size_t  pow2resize( size_t newsize );
size_t  arr_resize( size_t newsize );

// initialization -------------------------------------------------------------
void    memory_init( void );

#endif
