#ifndef rascal_type_h
#define rascal_type_h

#include "rascal.h"

/* global dispatch tables */
extern char  *TypeNames[NUM_TYPES];
extern bool (*HasType[NUM_TYPES])(Value x, RlType type);
extern void (*Print[NUM_TYPES])(Value x); 


#endif
