#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// external API
void    initEnvt(Vm* vm);
void    freeEnvt(Vm* vm);

#define getAnnot(x, k)    generic2(getAnnot, x, x, k)
#define setAnnot(x, k, v) generic2(setAnnot, x, k, v)

Value  getAnotVal(Value x, Value key);
Value  getAnnotObj(void* p, Value key);
Value  setAnnotVal(Value x, Value key, Value value);
Value  setAnnotObj(void* p, Value key, Value value);

size_t  defineGlobal(Vm* vm, Symbol* name, Value init, int flags);
bool    lookupGlobal(Vm* vm, Symbol* name, Value* buf);
Value   defineSpecial(char* name, CompileFn compile);

#endif
