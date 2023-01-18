#ifndef rascal_type_h
#define rascal_type_h

#include "rascal.h"

/* global dispatch tables */
extern char   *TypeNames[NUM_TYPES];
extern usize   BaseSize[NUM_TYPES];
extern uint16  Offset[NUM_TYPES];

extern bool  (*HasType[NUM_TYPES])(Value x, RlType type);
extern void  (*Print[NUM_TYPES])(Value x);
extern usize (*Alloc[NUM_TYPES])(RlType type, void *args, void **dst);
extern void  (*Init[NUM_TYPES])(void *self, RlType type, void *args);
extern void  (*Free[NUM_TYPES])(Object self);

#endif
