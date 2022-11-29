#ifndef rl_rl_h
#define rl_rl_h

#include "rl/compile.h"
#include "rl/eval.h"
#include "rl/apply.h"
#include "rl/exec.h"
#include "rl/prin.h"
#include "rl/read.h"

/* runtime */
void rl_rl_init( void );
void rl_rl_mark( void );

#endif
