#ifndef rascal_memory_h
#define rascal_memory_h

#include "rascal.h"

// flags stored for each 
enum memfl_t {
  mem_allocated  = 0x0010,
  mem_relocated  = 0x0020,
  mem_raw        = 0x0040,
  mem_blackbit   = 0x0080,

  // flags used only during allocation
  memfl_global   = 0x0100, // allocate globally
  memfl_unboxed  = 0x0200, // the initial values being supplied are unboxed
  memfl_sequence = 0x0400, // the arity is the number of nodes in an inductive sequence. If the array flag is not set, then each node wants count extra space
  memfl_array    = 0x0800, // the arity is the number of elements and the size is the size of each element
  memfl_shared   = 0x1000, // the requested object chares data with another, and only the base object size should be allocated
  memfl_assoc    = 0x2000  // the object is being used to store a key, value, and hash. count is the hash, and size is a count bytes being used for inline storage of the key
};

// exports --------------------------------------------------------------------
uchar_t *findmap( void_t *p );
bool_t   overflowp( size_t n );

uchar_t  getmemfl( void_t *ptr );
uchar_t  setmemfl( void_t *ptr, uchar_t fl );
uchar_t  clearmemfl( void_t *ptr, uchar_t fl );

void_t   *allocate( size_t nbytes, uint_t flags );
object_t *construct( type_t type, uint_t flags, size_t count, size_t size );
int_t     init( type_t type, uint_t flags, size_t count, size_t size, value_t ini, void *spc );

void    manage( void ); // GC entry point

void    relocate( value_t *buf, uchar_t **space, uchar_t **map, size_t *used ); // GC methods
value_t trace( value_t val );
void    untrace( value_t val );

size_t  pow2resize( size_t newsize );
size_t  arr_resize( size_t newsize );

// initialization -------------------------------------------------------------
void    initmemory( void );

#endif
