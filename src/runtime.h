#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "rascal.h"

/* forward declarations */
void *allocate(usize n);
void *allocArr(usize size, usize obsize);

void *reallocate(void *spc, usize oldsize, usize newsize);
void *reallocArr(void *spc, usize oldsize, usize newsize, usize obsize);

void  deallocate(void *spc, usize size);
void  deallocArr(void *spc, usize size, usize obsize);

#endif
