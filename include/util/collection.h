#ifndef rascal_util_collection_h
#define rascal_util_collection_h

#include "common.h"

/* general collection utilities */
bool   checkAlistResize(size_t newCnt, size_t oldCap, bool encoded);
bool   checkTableResize(size_t newCnt, size_t oldCap);

size_t padAlistSize(size_t oldCnt, size_t newCnt, size_t oldCap, bool encoded);
size_t padStackSize(size_t oldCnt, size_t newCnt, size_t oldCap, bool encoded);
size_t padOrdSize(size_t oldCnt, size_t newCnt, size_t oldCap);
size_t getOrdSize(size_t cnt);
void*  allocOrds(size_t cap, size_t oSize);

#endif
