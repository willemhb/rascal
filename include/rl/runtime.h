#ifndef rl_rl_h
#define rl_rl_h

/* toplevel */
#include "rl/compile.h"
#include "rl/eval.h"
#include "rl/apply.h"
#include "rl/exec.h"
#include "rl/prin.h"
#include "rl/read.h"
#include "rl/repl.h"

/* subdirectories */
#include "rl/readers/runtime.h"
#include "rl/native/runtime.h"
#include "rl/stx/runtime.h"

/* runtime */
void rl_rl_init( void );
void rl_rl_mark( void );
void rl_rl_cleanup( void );

#endif
