#ifndef rl_vm_environ_h
#define rl_vm_environ_h

#include "values.h"

/* Globals */
extern Env    Globals;
extern NSMap  NameSpaces;
extern SCache Strings;

/* External API */
void init_globals(Env* g);

#endif
