#ifndef rascal_util_hashing_h
#define rascal_util_hashing_h

#include "common.h"

// utilities
uint64_t ceilPow2(uint64_t word);

// hashing functions
uint64_t hashString(const char* chars);
uint64_t hashBytes(const uint8_t* bytes, size_t n);
uint64_t hashWord(uint64_t word);
uint64_t hashPtr(const void* pointer);
uint64_t hashDouble(double num);
uint64_t mixHashes(uint64_t hx, uint64_t hy);

#endif
