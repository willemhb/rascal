#ifndef rascal_utils_strutils_h
#define rascal_utils_strutils_h

#include "common.h"

// memcmp for different byte sizes
int  u16cmp( uint16_t *xb, uint16_t *yb, size_t n );
int  u32cmp( uint32_t *xb, uint32_t *yb, size_t n );
long u64cmp( uint64_t *xb, uint64_t *yb, size_t n );

// unicode utilities


#endif
