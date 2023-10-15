#ifndef rascal_native_h
#define rascal_native_h

#include "common.h"

// includes native function definitions, multimethod & macro internals, &c
void addMethod(Function* func, List* sig, Obj* method);
Obj* getMethod(Function* func, List* sig);

// external API
void initializeNativeFunctions(void);

#endif
