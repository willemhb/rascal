#ifndef rascal_native_h
#define rascal_native_h

#include "common.h"

// includes native function definitions, multimethod & macro internals, &c
Method* getMethod(Function* generic, Tuple* sig);
void    addMethod(Function* generic, Tuple* sig, Obj* specialized);


// external API
void initializeNativeFunctions(void);

#endif
