#ifndef rascal_hashing_h
#define rascal_hashing_h

#include "common.h"

uint64_t hashString(const char* chars);
uint64_t hashBytes(const uint8_t* bytes, size_t n);
uint64_t hashWord(uint64_t word);

#endif
