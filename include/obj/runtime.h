#ifndef rl_obj_runtime_h
#define rl_obj_runtime_h

/* object type includes */
#include "obj/bool.h"
#include "obj/closure.h"
#include "obj/cons.h"
#include "obj/control.h"
#include "obj/fixnum.h"
#include "obj/lambda.h"
#include "obj/native.h"
#include "obj/nul.h"
#include "obj/real.h"
#include "obj/stream.h"
#include "obj/symbol.h"
#include "obj/type.h"
#include "obj/symbol.h"

/* runtime */
void rl_obj_init( void );
void rl_obj_mark( void );
void rl_obj_cleanup( void );

#endif
