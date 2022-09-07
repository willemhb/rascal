#ifndef hashing_h
#define hashing_h

#include <stdint.h>
#include <stddef.h>

#define mixhash(a, b) int64hash((int64_t)(a) ^ (int64_t)(b))

uint32_t int32hash(uint32_t key);
uint32_t floathash(float key);
uint64_t int64hash(uint64_t key);
uint64_t doublehash(double key);
uint64_t pointerhash(void *key);

uint64_t memhash(const unsigned char *buf, size_t n );
uint64_t strhash( const char *buf );

#endif
