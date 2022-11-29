#ifndef rl_obj_obj_h
#define rl_obj_obj_h

/* object type includes */
#include "obj/control.h"
#include "obj/lambda.h"
#include "obj/nul.h"
#include "obj/real.h"
#include "obj/stream.h"
#include "obj/type.h"
#include "obj/symbol.h"
#include "obj/cons.h"
#include "obj/native.h"
#include "obj/bool.h"

/* runtime */
void rl_obj_init( void );
void rl_obj_mark( void );

#endif
