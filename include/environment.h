#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// external API
void    initEnvt(Vm* vm);
void    freeEnvt(Vm* vm);

Symbol* internSymbol(Vm* vm, char* name);
size_t  defineGlobal(Vm* vm, Symbol* name, Value init);
bool    lookupGlobal(Vm* vm, Symbol* name, Value* buf);
Value   defineSpecial(char* name, CompileFn compile);

#endif
