#ifndef rascal_native_h
#define rascal_native_h

#include "common.h"

// includes native function definitions, multimethod & macro internals, &c
Method* getMethod(Function* g, Tuple* s);
void    addMethod(Function* g, Tuple* s, Obj* m, bool va);

// external API
void initializeNativeFunctions(void);

#endif
